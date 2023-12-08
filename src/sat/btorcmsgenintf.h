#ifndef BTORCMSGENINTF_H_INCLUDED
#define BTORCMSGENINTF_H_INCLUDED

#include "cmsgen/cmsgen.h"
#include "boolector.h"

extern "C" {
  
  using namespace CMSGen;
  
  void set_sampling_node (Btor *btor, BoolectorNode *node);
  // void set_cmsgen_sampling (Btor *btor, uint32_t *vars, uint32_t n_vars);
  void cmsgen_resample (Btor *btor);
  void set_cmsgen_seed (Btor *btor, uint32_t *seed);
  // const lbool* get_cmsgen_model (Btor *btor);
  uint64_t get_assignment (Btor *btor, uint32_t dom_idx);
  
}

#endif
