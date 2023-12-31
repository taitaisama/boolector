
#ifdef BTOR_USE_CMSGEN

#include "sat/btorgenintf.h"
#include <algorithm>
#include <cmsgen/cmsgen.h>

extern "C" {
  
#include "btorabort.h"
#include "btorsat.h"
#include "btorcore.h"
#include "btoropt.h"
#include "btoraigvec.h"
#include "sat/btorcmsgen.h"
  
}
  
using namespace CMSGen;

/*------------------------------------------------------------------------*/


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

BtorCMSGen::BtorCMSGen (Btor *b, uint32_t *seed) : SATSolver(NULL, NULL, seed), BtorGenIntf(b), size (0), failed_map (0), assigned_map (0), nomodel (true) {}

BtorCMSGen::~BtorCMSGen () { reset (); }

int32_t BtorCMSGen::inc ()
{
  nomodel = true;
  new_var ();
  model.push_back(false);
  return nVars ();
}

inline Lit convert (CMSat::Lit lit) {
  return Lit(lit.var(), lit.sign());
}

void BtorCMSGen::assume (int32_t lit)
{
  nomodel = true;
  assumptions.push_back (convert(import (lit)));
}

void BtorCMSGen::add (int32_t lit)
{
  nomodel = true;
  if (lit)
    clause.push_back (convert(import (lit)));
  else
    add_clause (clause), clause.clear ();
}

int32_t BtorCMSGen::sat ()
{
  calls++;
  reset ();
  set_sampl_vars();
  lbool res = solve (&assumptions, (sampl_vars.size() > 0));
  const std::vector<lbool> &v =  get_model();
  for (int i = 0; i < v.size(); i ++) {
    model[i] = v[i] == l_True;
  }
  analyze_fixed ();
  conflicts += get_last_conflicts ();
  decisions += get_last_decisions ();
  propagations += get_last_propagations ();
  old_assumptions = assumptions;
  assumptions.clear ();
  nomodel = res != l_True;
  return res == l_Undef ? 0 : (res == l_True ? 10 : 20);
}

int32_t BtorCMSGen::failed (int32_t lit)
{
  if (!failed_map) analyze_failed ();
  Lit l      = convert(import (lit));
  uint32_t v = l.var ();
  assert (v < nVars ());
  return failed_map[v];
}

int32_t BtorCMSGen::fixed (int32_t lit)
{
  Lit l      = convert(import (lit));
  uint32_t v = l.var ();
  if (v >= size) return 0;
  return l.sign () ? -assigned_map[v] : assigned_map[v];
}

int32_t BtorCMSGen::deref (int32_t lit)
{
  if (nomodel) return fixed (lit);
  const std::vector<lbool>& model = get_model ();
  Lit l                           = convert(import (lit));
  uint32_t v                      = l.var ();
  assert (v < model.size ());
  int32_t res = model[l.var ()] == l_True ? 1 : -1;
  return l.sign () ? -res : res;
}

void BtorCMSGen::resample() {
  solve(&(old_assumptions), (sampl_vars.size() > 0));
  const std::vector<lbool> &v =  get_model();
  for (int i = 0; i < v.size(); i ++) {
    model[i] = v[i] == l_True;
  }
  
}

const std::vector<bool>& BtorCMSGen::get_gen_model() {
  
  return model;
  
}

void BtorCMSGen::set_gen_sampling_vars(std::vector<uint32_t> *sampling_vars) {
  
  set_sampling_vars(sampling_vars);
  
}

void BtorCMSGen::multisample(uint32_t count, void(*callback)(void*), void* data) {

  for (int i = 0; i < count; i ++) {
    resample();
    callback(data);
  }
  
}
  
/*------------------------------------------------------------------------*/

static void*
init (BtorSATMgr* smgr)
{
  (void) smgr;
  // uint32_t nthreads;
  uint32_t seed = btor_opt_get (smgr->btor, BTOR_OPT_SEED);
  BtorCMSGen* res = new BtorCMSGen (smgr->btor, &seed);
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
      "[cmsgen] calls %llu\n"
      "[cmsgen] conflicts %llu\n"
      "[cmsgen] decisions %llu\n"
      "[cmsgen] propagations %llu\n",
      (unsigned long long) solver->calls,
      (unsigned long long) solver->conflicts,
      (unsigned long long) solver->decisions,
      (unsigned long long) solver->propagations);
  fflush (stdout);
}

/*------------------------------------------------------------------------*/  

extern "C" {
  
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
  
};

#endif
