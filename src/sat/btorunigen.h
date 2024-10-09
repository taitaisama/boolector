#ifndef BTORUNIGEN_H_INCLUDED
#define BTORUNIGEN_H_INCLUDED

/*------------------------------------------------------------------------*/
#ifdef BTOR_USE_UNIGEN
/*------------------------------------------------------------------------*/

#include "btorsat.h"

bool btor_sat_enable_unigen (BtorSATMgr* smgr);

#define BTORUNIGEN_SAMPLE_COUNT 16

/*------------------------------------------------------------------------*/
#endif
/*------------------------------------------------------------------------*/

#endif
