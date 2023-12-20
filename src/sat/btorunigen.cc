
#ifdef BTOR_USE_UNIGEN

#include "sat/btorgenintf.h"
#include <algorithm>


extern "C" {
  
#include "btorabort.h"
#include "btorsat.h"
#include "btorcore.h"
#include "btoropt.h"
#include "btoraigvec.h"


#include "sat/btorunigen.h"

}
  
using namespace UniGen;
using namespace ApproxMC;

  
/*------------------------------------------------------------------------*/

void set_model(const std::vector<int>& s, void* x) {

  BtorUniGen* btorunigen = (BtorUniGen*) x;
  
  for (auto u: s) {
    if (u < 0) {
      btorunigen->model[-u+1] = false;
    }
    else {
      btorunigen->model[u-1] = true;
    }
  }
  
}

BtorUniGen::BtorUniGen (Btor *b, uint32_t *seed) : BtorGenIntf(b) {
  appmc = new ApproxMC::AppMC;
  appmc->set_seed(*seed);
  unig = new UniG(appmc);
}

BtorUniGen::~BtorUniGen () {}

// Btorunigen::~BtorUniGen () {}

int32_t BtorUniGen::inc ()
{
  appmc->new_var ();
  model.push_back(false);
  return appmc->nVars ();
}

void BtorUniGen::add (int32_t lit)
{
  if (lit)
    clause.push_back (import (lit));
  else
    appmc->add_clause (clause), clause.clear ();
}

void BtorUniGen::sat ()
{
  calls ++;
  set_sampl_vars();
  unig->sample(&sol_count, 1);
}

void BtorUniGen::set_gen_sampling_vars(std::vector<uint32_t> *sampling_vars) {

  appmc->set_projection_set(*sampling_vars);
  sol_count = appmc->count();
  unig->set_full_sampling_vars(*sampling_vars);
  unig->set_callback(set_model, this);
  
}
  
void BtorUniGen::resample() {

  unig->sample(&sol_count, 1);
  
}

const std::vector<bool>& BtorUniGen::get_gen_model() {

  return model;
  
}

void BtorUniGen::multisample(uint32_t count, void(*cb)(void)) {

  auto lambda_callback = [&](const std::vector<int>& s, void* btorunigen) {
    set_model(s, btorunigen);
    cb();
  };

  unig->set_callback(lambda_callback, this);
  unig->sample(&sol_count, count);
  
}


  
/*------------------------------------------------------------------------*/

static void*
init (BtorSATMgr* smgr)
{
  (void) smgr;
  // uint32_t nthreads;
  uint32_t seed = btor_opt_get (smgr->btor, BTOR_OPT_SEED);
  BtorUniGen* res = new BtorUniGen (smgr->btor, &seed);
  // if ((nthreads = btor_opt_get(smgr->btor, BTOR_OPT_SAT_ENGINE_N_THREADS)) > 1)
  //   res->set_num_threads(nthreads);
  return res;
}

static void
add (BtorSATMgr* smgr, int32_t lit)
{
  BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  solver->add (lit);
}

static int32_t
sat (BtorSATMgr* smgr, int32_t limit)
{
  (void) limit;
  BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  solver->sat ();
  return 10;
}

static int32_t
deref (BtorSATMgr* smgr, int32_t lit)
{

  assert(false);
  // BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  // return solver->deref (lit);
}

static void
reset (BtorSATMgr* smgr)
{
  BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  delete solver;
}

static int32_t
inc_max_var (BtorSATMgr* smgr)
{
  BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  return solver->inc ();
}

static void
assume (BtorSATMgr* smgr, int32_t lit)
{
  assert(false);
  // BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  // solver->assume (lit);
}

static int32_t
fixed (BtorSATMgr* smgr, int32_t lit)
{
  assert(false);
  // BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  // return solver->fixed (lit);
}

static int32_t
failed (BtorSATMgr* smgr, int32_t lit)
{
  assert(false);
  // BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  // return solver->failed (lit);
}

static void
enable_verbosity (BtorSATMgr* smgr, int32_t level)
{
  // (void) smgr;
  // if (level >= 2) ((BtorUniGen*) smgr->solver)->set_verbosity (level - 1);
}

static void
stats (BtorSATMgr* smgr)
{
  BtorUniGen* solver = (BtorUniGen*) smgr->solver;
  printf (
	  "[unigen] calls %llu\n", 
	  (unsigned long long) solver->calls);
  fflush (stdout);
}

/*------------------------------------------------------------------------*/  

extern "C" {
  
bool
btor_sat_enable_unigen (BtorSATMgr* smgr)
{
  assert (smgr != NULL);

  BTOR_ABORT (smgr->initialized,
              "'btor_sat_init' called before 'btor_sat_enable_unigen'");

  smgr->name = "UniGen";

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
