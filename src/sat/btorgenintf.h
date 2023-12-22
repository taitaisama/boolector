#ifndef BTORGENINTF_H_INCLUDED
#define BTORGENINTF_H_INCLUDED


#include "cryptominisat5/solvertypesmini.h"
#include <cmsgen/cmsgen.h>
#include <unigen/unigen.h>
#include <approxmc/approxmc.h>
#include <vector>

extern "C" {
  
#include "boolector.h"

  void set_sampling_node (Btor *btor, BoolectorNode *node);
  void resample (Btor *btor);
  uint64_t get_assignment (Btor *btor, uint32_t dom_idx);
  void configure_sat (Btor *btor);
  void multisample (Btor *btor, uint32_t count, void(*callback)(void*), void* data);
  
};
    
class BtorGenIntf {

  Btor *btor;

public:

  BtorGenIntf(Btor* b);
  virtual ~BtorGenIntf();
    
  std::vector<BoolectorNode*> sampling_nodes;
  std::vector<std::vector<CMSat::Lit>> sampling_lits;
  std::vector<uint32_t> sampl_vars;
    
  CMSat::Lit import (int32_t lit);
  void set_sampl_vars();
  void set_sampling_lits (BoolectorNode *node);
    
  virtual void resample() = 0;
  virtual void multisample(uint32_t count, void(*callback)(void*), void* data) = 0;
  virtual const std::vector<bool>& get_gen_model() = 0;
  virtual void set_gen_sampling_vars(std::vector<uint32_t> *sampling_vars) = 0;

  friend void set_sampling_node (Btor *btor, BoolectorNode *node);
  friend void resample (Btor *btor);
  friend uint64_t get_assignment (Btor *btor, uint32_t dom_idx);
};

class BtorCMSGen : public CMSGen::SATSolver, public BtorGenIntf {

  uint32_t size;
  std::vector<CMSGen::Lit> assumptions, clause, old_assumptions;
  std::vector<bool> model;
  signed char* failed_map;
  int32_t* assigned_map;
  bool nomodel;
  
  void reset ();
  void analyze_failed ();
  void analyze_fixed ();
  
public:
  BtorCMSGen (Btor *b, uint32_t *seed);
  ~BtorCMSGen ();

  void resample();
  void multisample(uint32_t count, void(*callback)(void*), void* data);
  const std::vector<bool>& get_gen_model();
  void set_gen_sampling_vars(std::vector<uint32_t> *sampling_vars);
  
  int32_t inc ();
  void assume (int32_t lit);
  void add (int32_t lit);
  int32_t sat ();
  int32_t failed (int32_t lit);
  int32_t fixed (int32_t lit);
  int32_t deref (int32_t lit);

  uint64_t calls, conflicts, decisions, propagations;
};

class BtorUniGen : public BtorGenIntf {

public:
  
  std::vector<CMSat::Lit> clause;
  std::vector<bool> model;
  ApproxMC::AppMC* appmc;
  UniGen::UniG* unig;
  ApproxMC::SolCount sol_count;
  
  BtorUniGen (Btor *b, uint32_t *seed);
  ~BtorUniGen ();
  
  void resample();
  void multisample(uint32_t count, void(*callback)(void*), void* data);
  const std::vector<bool>& get_gen_model();
  void set_gen_sampling_vars(std::vector<uint32_t> *sampling_vars);
  
  int32_t inc ();
  void add (int32_t lit);
  void sat ();

  uint64_t calls;
};

#endif
