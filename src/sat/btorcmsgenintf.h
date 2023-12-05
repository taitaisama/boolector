#ifndef BTORCMSGENINTF_H_INCLUDED
#define BTORCMSGENINTF_H_INCLUDED


#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>
#include "cmsgen/cmsgen.h"
#include "btorsat.h"

extern "C" {
  
using namespace CMSGen;
  
class BtorCMSGen : public SATSolver {

  uint32_t size;
  std::vector<Lit> assumptions, clause;
  std::vector<std::vector<Lit>> sampling_nodes;
  std::vector<uint32_t> sampling_vars;
  signed char* failed_map;
  int32_t* assigned_map;
  bool nomodel;
  
  Lit import (int32_t lit);
  void reset ();
  void analyze_failed ();
  void analyze_fixed ();
  
 public:
  BtorCMSGen (uint32_t* seed);
  ~BtorCMSGen ();
  int32_t inc ();
  void assume (int32_t lit);
  void add (int32_t lit);
  int32_t sat ();
  int32_t failed (int32_t lit);
  int32_t fixed (int32_t lit);
  int32_t deref (int32_t lit);
  
  void add_sampling_node (Btor *btor, BoolectorNode *node);
  void set_sampling();
  void resample (bool only_sampling);
  void get_assignments (std::vector<std::vector<bool>> &vars);
  

  uint64_t calls, conflicts, decisions, propagations;
  
};
  
BtorCMSGen *get_cmsgen_solver (Btor *btor);

}

#endif
