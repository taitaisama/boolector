#ifndef BTORCMSGENINTF_H_INCLUDED
#define BTORCMSGENINTF_H_INCLUDED

#include "cmsgen/cmsgen.h"
#include "boolector.h"

extern "C" {
  
  using namespace CMSGen;
  
  void get_node_literals (Btor *btor, BoolectorNode *node, Lit *lits);
  void set_cmsgen_sampling (Btor *btor, uint32_t *vars, uint32_t n_vars);
  void cmsgen_resample (Btor *btor, bool only_sampling);
  void set_cmsgen_seed (Btor *btor, uint32_t *seed);
  const lbool* get_cmsgen_model (Btor *btor);
  
}

#endif
