#ifndef _OMV_PROCS_H_
#define _OMV_PROCS_H_

#include "stg.h"

#define OMV_EMPTY                    STG_EMPTY
#define OMV_ALL                      STG_ALL
#define OMV_TEST                     0x40000000

#define OMV_INPUT                    STG_INPUT
#define OMV_OUTPUT                   STG_OUTPUT
#define OMV_INTERNAL                 STG_INTERNAL
#define OMV_DUMMY                    STG_DUMMY

#define OMV_DC                       STG_PLACE
#define OMV_FF                       STG_TRANSITION
#define OMV_FF_INPUT                 (OMV_FF | OMV_INPUT)
#define OMV_FF_OUTPUT                (OMV_FF | OMV_OUTPUT)
#define OMV_FF_INTERNAL              (OMV_FF | OMV_INTERNAL)
#define OMV_FF_DUMMY                 (OMV_FF | OMV_DUMMY)

struct omv_t
{
  // List of DC
  // data->type    - type of DC
  // data->name    - name of DC
  // data->comment - type of DC
  // data->pred    - list of request signals (in order of inputs in type of DC)
  // data->succ    - list of acknowledgement signals (in order of inputs in type of DC)
  // data->link    - reserved (NULL)
  // index_0       - reserved
  // index_1       - reserved
  PNode dc;

  // List of FF
  // data->type    - type of FF
  // data->name    - name of FF
  // data->comment - type of FF
  // data->pred    - list of set inputs (in order of inputs in type of FF)
  // data->succ    - list of reset inputs (in order of inputs in type of FF)
  // data->link    - reserved (NULL)
  // index_0       - reserved
  // index_1       - reserved
  PNode ff;

  // List of test signals
  // data->type    - type of a signal
  // data->name    - name of a signal
  // data->comment - comment for a signal
  // data->pred    - reserved (NULL)
  // data->succ    - reserved (NULL)
  // data->link    - pointer to a initial state (i) node for a signal
  // index_0       - max index of minus transition (or dummy)
  // index_1       - max index of plus transition
  PNode tst;

  // name of STG
  char *model_name;
};

extern omv_t *omv_create ();
extern int omv_destroy (omv_t *omv);
extern int omv_sort (omv_t *omv);
extern int omv_calculate (stg_t *stg, omv_t *omv, int flag = (OMV_DC | OMV_FF));
extern int omv_info_write (FILE *outfile, omv_t *omv, int flag = (OMV_DC | OMV_FF));
extern int omv_verilog_write (FILE *outfile, stg_t *stg, omv_t *omv, int flag = (OMV_DC | OMV_FF));
extern int omv_debug (FILE *outfile, omv_t *omv, int flag = (OMV_DC | OMV_FF));

#endif
