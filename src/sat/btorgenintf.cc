
#include "sat/btorgenintf.h"
#include <algorithm>
#include <vector>

extern "C" {
  
#include "btorabort.h"
#include "btorsat.h"
#include "btorcore.h"
#include "btoropt.h"
#include "btoraigvec.h"

}

/*------------------------------------------------------------------------*/

BtorGenIntf::BtorGenIntf(Btor *b) {
  btor = b;
}

BtorGenIntf::~BtorGenIntf() { }

void BtorGenIntf::set_sampling_lits (BoolectorNode *node)
{
  
  BtorNode *exp, *real_exp;
  bool inv;
  BtorAIGVec *av;
  uint32_t i, width;
  int32_t lit;
  CMSat::Lit l;
  BtorAIG *aig;
  sampling_lits.push_back(std::vector<CMSat::Lit>());
  std::vector<CMSat::Lit> &v = sampling_lits[sampling_lits.size()-1];
  
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
      l = CMSat::Lit(0, (BTOR_IS_INVERTED_AIG (aig) ^ inv));
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

void BtorGenIntf::set_sampl_vars()
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
  set_gen_sampling_vars(&sampl_vars);
}

CMSat::Lit BtorGenIntf::import (int32_t lit)
{
  // assert (0 < abs (lit) && ((uint32_t) abs (lit)) <= nVars ());
  return CMSat::Lit (abs (lit) - 1, lit < 0);
}

/*------------------------------------------------------------------------*/  


extern "C" {
  
BtorGenIntf *get_gen_solver (Btor *btor) {
  
  BtorSATMgr* smgr;
  
  smgr = btor_get_sat_mgr (btor);
  assert (smgr != NULL);
  
  if (strcmp(smgr->name, "CMSGen") == 0) {
    return (BtorGenIntf*) ((BtorCMSGen*) smgr->solver);
  }
  else if (strcmp(smgr->name, "UniGen") == 0){
    return (BtorGenIntf*) ((BtorUniGen*) smgr->solver);
  }
  assert(false);
  
}  
  
void set_sampling_node (Btor *btor, BoolectorNode *node) {

  BtorGenIntf* GenSolver = get_gen_solver (btor);
  GenSolver->sampling_nodes.push_back(node);  

}

void resample (Btor *btor) {

  BtorGenIntf* GenSolver = get_gen_solver (btor);
  GenSolver->resample();
  
}

void multisample (Btor *btor, uint32_t count, void(*callback)(void*), void* data) {

  BtorGenIntf* GenSolver = get_gen_solver (btor);
  GenSolver->multisample(count, callback, data);
  
}

void configure_sat_mgr (Btor *btor);
  
void configure_sat (Btor *btor) {

  configure_sat_mgr(btor);
  
}

uint64_t get_assignment (Btor *btor, uint32_t dom_idx) {

  BtorGenIntf* GenSolver = get_gen_solver (btor);
  const std::vector<bool> &model = GenSolver->get_gen_model();
  assert(GenSolver->sampling_nodes.size() > dom_idx);
  std::vector<CMSat::Lit> &v = GenSolver->sampling_lits[dom_idx];
  uint64_t ret = 0;
  
  for (uint32_t i = 0, j = v.size()-1; i < v.size(); i ++, j--) {
    uint32_t var = v[i].var();
    bool val;
    if (var) {
      val = model[var];
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

