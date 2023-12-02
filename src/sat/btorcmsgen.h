/*  Boolector: Satisfiability Modulo Theories (SMT) solver.
 *
 *  Copyright (C) 2007-2021 by the authors listed in the AUTHORS file.
 *
 *  This file is part of Boolector.
 *  See COPYING for more information on using this software.
 */

#ifndef BTORCMSGEN_H_INCLUDED
#define BTORCMSGEN_H_INCLUDED

/*------------------------------------------------------------------------*/
#ifdef BTOR_USE_CMSGEN
/*------------------------------------------------------------------------*/

#include "btorsat.h"

bool btor_sat_enable_cmsgen (BtorSATMgr* smgr);

/*------------------------------------------------------------------------*/
#endif
/*------------------------------------------------------------------------*/

#endif
