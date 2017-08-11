#ifndef _OMD_PROCS_H_
#define _OMD_PROCS_H_

#include <stdio.h>
#include "stg.h"

#define OMD_NAME_SEP      ','

extern int omd_mandatory (stg_t *stg);
extern int omd_force_class (stg_t *stg, int start, int end, int fork, int join, int choice, int merge, int mark);
extern int omd_force_list (stg_t *stg, char *redundant_names, char *separator_names, int num_loop);
extern int omd_eliminate_input_output (stg_t *stg, int num_loop);
extern int omd_eliminate_chain (stg_t *stg, int num_loop);
extern int omd_eliminate_one (stg_t *stg, int num_loop);

#endif
