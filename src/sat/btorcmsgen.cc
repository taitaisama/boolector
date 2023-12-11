
#ifdef BTOR_USE_CMSGEN

#include "sat/btorcmsgenintf.h"
#include <algorithm>

extern "C" {
  
#include "btorabort.h"
#include "btorsat.h"
#include "btorcore.h"
#include "btoropt.h"
#include "btoraigvec.h"
#include "sat/btorcmsgen.h"
  
using namespace CMSGen;
  
class BtorCMSGen : public SATSolver {

  uint32_t size;
  std::vector<Lit> assumptions, clause;
  std::vector<BoolectorNode*> sampling_nodes;
  std::vector<std::vector<Lit>> sampling_lits;
  std::vector<uint32_t> sampl_vars;
  signed char* failed_map;
  int32_t* assigned_map;
  bool nomodel;
  Btor* btor;
  
  Lit import (int32_t lit);
  void reset ();
  void analyze_failed ();
  void analyze_fixed ();
  void set_sampl_vars();
  void set_sampling_lits (BoolectorNode *node);
  
 public:
  BtorCMSGen (Btor* b);
  ~BtorCMSGen ();
  int32_t inc ();
  void assume (int32_t lit);
  void add (int32_t lit);
  int32_t sat ();
  int32_t failed (int32_t lit);
  int32_t fixed (int32_t lit);
  int32_t deref (int32_t lit);

  uint64_t calls, conflicts, decisions, propagations;

  friend void set_sampling_node (Btor *btor, BoolectorNode *node);
  friend void cmsgen_resample (Btor *btor);
  friend uint64_t get_assignment (Btor *btor, uint32_t dom_idx);
};

/*------------------------------------------------------------------------*/

void BtorCMSGen::set_sampling_lits (BoolectorNode *node)
{
  
  BtorNode *exp, *real_exp;
  bool inv;
  BtorAIGVec *av;
  uint32_t i, width;
  int32_t lit;
  Lit l;
  BtorAIG *aig;
  sampling_lits.push_back(std::vector<Lit>());
  std::vector<Lit> &v = sampling_lits[sampling_lits.size()-1];
  
  exp = BTOR_IMPORT_BOOLECTOR_NODE (node);
  exp = btor_node_get_simplified(btor, exp);
  real_exp = btor_node_real_addr (exp);
  av = real_exp->av;

  // turn simplification off to avoid av == null
  assert(av);
  
  width = av->width;
  inv = btor_node_is_inverted (exp);
  
  for (i = 0; i < width; i++)
  {
    aig = av->aigs[i];
    if (aig == BTOR_AIG_TRUE || aig == BTOR_AIG_FALSE) {
      // turn simplification off to avoid this case
      l = Lit(0, (BTOR_IS_INVERTED_AIG (aig) ^ inv));
    }
    else {
      lit = BTOR_REAL_ADDR_AIG (aig)->cnf_id;
      assert (lit > 0);
      sampl_vars.push_back(lit);
      l = import (lit);
      l ^= (BTOR_IS_INVERTED_AIG (aig) ^ inv);
    }
    v.push_back(l);
  }  
}

void BtorCMSGen::set_sampl_vars()
{

  if (sampling_nodes.size() == 0)
    return;
  sampl_vars.clear();
  for (BoolectorNode * node: sampling_nodes) {
    set_sampling_lits(node);
  }
  if (sampl_vars.size() == 0)
    return;
  std::sort(sampl_vars.begin(), sampl_vars.end());
  std::vector<uint32_t> temp;
  temp.push_back(sampl_vars[0]);
  for (uint32_t i = 1; i < sampl_vars.size(); i ++) {
    if (temp[temp.size()-1] != sampl_vars[i]) {
      temp.push_back(sampl_vars[i]);
    }
  }
  std::swap(sampl_vars, temp);
  set_sampling_vars(&sampl_vars);
}

Lit BtorCMSGen::import (int32_t lit)
{
  assert (0 < abs (lit) && ((uint32_t) abs (lit)) <= nVars ());
  return Lit (abs (lit) - 1, lit < 0);
}

void BtorCMSGen::reset ()
{
  if (failed_map) delete[] failed_map, failed_map = 0;
  if (assigned_map) delete[] assigned_map, assigned_map = 0;
  size = 0;
}

void BtorCMSGen::analyze_failed ()
{
  uint32_t nvars = nVars ();
  failed_map     = new signed char[nvars];
  memset (failed_map, 0, nvars);
  const std::vector<Lit>& conflict = get_conflict ();
  for (size_t i = 0; i < conflict.size (); i++)
    {
      uint32_t v = conflict[i].var ();
      assert (v < nvars);
      failed_map[v] = 1;
    }
}

void BtorCMSGen::analyze_fixed ()
{
  assert (!assigned_map);
  assert (!size);
  size         = nVars ();
  assigned_map = new int32_t[size];
  memset (assigned_map, 0, sizeof(int32_t) * size);
  const std::vector<Lit> assigned_lits = get_zero_assigned_lits ();
  for (size_t i = 0; i < assigned_lits.size (); i++)
    {
      Lit l      = assigned_lits[i];
      uint32_t v = l.var ();
      assert (v < size);
      assigned_map[v] = l.sign () ? -1 : 1;
    }
}

BtorCMSGen::BtorCMSGen (Btor *b) : size (0), failed_map (0), assigned_map (0), nomodel (true), btor(b) {}

BtorCMSGen::~BtorCMSGen () { reset (); }

int32_t BtorCMSGen::inc ()
{
  nomodel = true;
  new_var ();
  return nVars ();
}

void BtorCMSGen::assume (int32_t lit)
{
  nomodel = true;
  assumptions.push_back (import (lit));
}

void BtorCMSGen::add (int32_t lit)
{
  nomodel = true;
  if (lit)
    clause.push_back (import (lit));
  else
    add_clause (clause), clause.clear ();
}

int32_t BtorCMSGen::sat ()
{
  calls++;
  reset ();
  set_sampl_vars();
  lbool res = solve (&assumptions, (sampl_vars.size() > 0));
  analyze_fixed ();
  conflicts += get_last_conflicts ();
  decisions += get_last_decisions ();
  propagations += get_last_propagations ();
  assumptions.clear ();
  nomodel = res != l_True;
  return res == l_Undef ? 0 : (res == l_True ? 10 : 20);
}

int32_t BtorCMSGen::failed (int32_t lit)
{
  if (!failed_map) analyze_failed ();
  Lit l      = import (lit);
  uint32_t v = l.var ();
  assert (v < nVars ());
  return failed_map[v];
}

int32_t BtorCMSGen::fixed (int32_t lit)
{
  Lit l      = import (lit);
  uint32_t v = l.var ();
  if (v >= size) return 0;
  return l.sign () ? -assigned_map[v] : assigned_map[v];
}

int32_t BtorCMSGen::deref (int32_t lit)
{
  if (nomodel) return fixed (lit);
  const std::vector<lbool>& model = get_model ();
  Lit l                           = import (lit);
  uint32_t v                      = l.var ();
  assert (v < model.size ());
  int32_t res = model[l.var ()] == l_True ? 1 : -1;
  return l.sign () ? -res : res;
}
  
/*------------------------------------------------------------------------*/

static void*
init (BtorSATMgr* smgr)
{
  (void) smgr;
  // uint32_t nthreads;
  BtorCMSGen* res = new BtorCMSGen (smgr->btor);
  // if ((nthreads = btor_opt_get(smgr->btor, BTOR_OPT_SAT_ENGINE_N_THREADS)) > 1)
  //   res->set_num_threads(nthreads);
  return res;
}

static void
add (BtorSATMgr* smgr, int32_t lit)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  solver->add (lit);
}

static int32_t
sat (BtorSATMgr* smgr, int32_t limit)
{
  (void) limit;
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  return solver->sat ();
}

static int32_t
deref (BtorSATMgr* smgr, int32_t lit)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  return solver->deref (lit);
}

static void
reset (BtorSATMgr* smgr)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  delete solver;
}

static int32_t
inc_max_var (BtorSATMgr* smgr)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  return solver->inc ();
}

static void
assume (BtorSATMgr* smgr, int32_t lit)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  solver->assume (lit);
}

static int32_t
fixed (BtorSATMgr* smgr, int32_t lit)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  return solver->fixed (lit);
}

static int32_t
failed (BtorSATMgr* smgr, int32_t lit)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  return solver->failed (lit);
}

static void
enable_verbosity (BtorSATMgr* smgr, int32_t level)
{
  (void) smgr;
  if (level >= 2) ((BtorCMSGen*) smgr->solver)->set_verbosity (level - 1);
}

static void
stats (BtorSATMgr* smgr)
{
  BtorCMSGen* solver = (BtorCMSGen*) smgr->solver;
  printf (
      "[cms] calls %llu\n"
      "[cms] conflicts %llu\n"
      "[cms] decisions %llu\n"
      "[cms] propagations %llu\n",
      (unsigned long long) solver->calls,
      (unsigned long long) solver->conflicts,
      (unsigned long long) solver->decisions,
      (unsigned long long) solver->propagations);
  fflush (stdout);
}

/*------------------------------------------------------------------------*/  
  
bool
btor_sat_enable_cmsgen (BtorSATMgr* smgr)
{
  assert (smgr != NULL);

  BTOR_ABORT (smgr->initialized,
              "'btor_sat_init' called before 'btor_sat_enable_cmsgen'");

  smgr->name = "CMSGen";

  BTOR_CLR (&smgr->api);
  smgr->api.add              = add;
  smgr->api.assume           = assume;
  smgr->api.deref            = deref;
  smgr->api.enable_verbosity = enable_verbosity;
  smgr->api.failed           = failed;
  smgr->api.fixed            = fixed;
  smgr->api.inc_max_var      = inc_max_var;
  smgr->api.init             = init;
  smgr->api.repr             = 0;
  smgr->api.reset            = reset;
  smgr->api.sat              = sat;
  smgr->api.set_output       = 0;
  smgr->api.set_prefix       = 0;
  smgr->api.stats            = stats;
  return true;
}

/*------------------------------------------------------------------------*/

    
BtorCMSGen *get_cmsgen_solver (Btor *btor) {
  
  BtorSATMgr* smgr;
    
  smgr = btor_get_sat_mgr (btor);
  assert (smgr != NULL);
  assert (strcmp(smgr->name, "CMSGen") == 0);
  
  return (BtorCMSGen*) smgr->solver;
}  
  
void set_sampling_node (Btor *btor, BoolectorNode *node) {

  BtorCMSGen* CMSGenSolver = get_cmsgen_solver (btor);
  CMSGenSolver->sampling_nodes.push_back(node);  

}

void cmsgen_resample (Btor *btor) {
  
  BtorCMSGen* CMSGenSolver;

  CMSGenSolver = get_cmsgen_solver (btor);
  CMSGenSolver->solve(&(CMSGenSolver->assumptions), (CMSGenSolver->sampl_vars.size() > 0));
  
}

void configure_sat (btor *btor) {
  
  BtorSATMgr *smgr;
  smgr = btor_get_sat_mgr (btor);
  if (btor_sat_is_initialized (smgr)) return;
  btor_sat_enable_solver (smgr);
  btor_sat_init (smgr);
  
}

// TODO
void set_cmsgen_seed (Btor *btor, uint32_t *seed) {
  
  assert(false);
  
}

uint64_t get_assignment (Btor *btor, uint32_t dom_idx) {

  BtorCMSGen* CMSGenSolver = get_cmsgen_solver (btor);
  const std::vector<lbool> &model = CMSGenSolver->get_model();
  assert(CMSGenSolver->sampling_nodes.size() > dom_idx);
  std::vector<Lit> &v = CMSGenSolver->sampling_lits[dom_idx];
  uint64_t ret = 0;
  
  for (uint32_t i = 0, j = v.size()-1; i < v.size(); i ++, j--) {
    uint32_t var = v[i].var();
    bool val;
    if (var) {
      val = model[var] == l_True ? true : false;
      val = v[i].sign() ? !val : val;
    }
    else {
      // turn off simplification to avoid this case
      val = v[i].sign();
    }
    if (val) {
      ret |= (1 << j);
    }
  }
  return ret;
}

};

#endif
