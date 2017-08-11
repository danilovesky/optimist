#ifndef _OMG_PROCS_H_
#define _OMG_PROCS_H_

extern int dot_write (FILE *outfile, stg_t *stg, int legend = 0, int connect = 0, int inputs = 0);
extern int dot_execute (stg_t *stg);

#endif
