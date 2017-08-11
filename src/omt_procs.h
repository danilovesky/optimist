#ifndef _OMT_PROCS_H_
#define _OMT_PROCS_H_

#include "stg.h"

extern int omt_fanin (stg_t *stg);
extern int omt_context (stg_t *stg);
extern int omt_marking (stg_t *stg);
extern int omt_redundant (stg_t *stg);
extern int omt_compress (stg_t *stg);
extern void omt_simplify (stg_t *stg, int rate);

#endif
