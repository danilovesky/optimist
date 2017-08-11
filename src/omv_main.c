#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "stg.h"
#include "stg_procs.h"
#include "omv_params.h"
#include "omv_procs.h"

int main (int argc, char **argv)
{
  stg_t *stg;
  omv_t *omv;
  int flag;

  stg = stg_create ();
  omv = omv_create ();

  omv_process_params (argc, argv);

  // input file
  if (_omv_in_file_name != NULL)
  {
    if ((_omv_in_file = fopen (_omv_in_file_name, "r")) == NULL)
     Fatal("opening input file <%s>", _omv_in_file_name);
  }
  else
    _omv_in_file = stdin;
  stg_read (_omv_in_file, stg);
  fclose (_omv_in_file);
  stg_compress(stg);

  // === general options ===
  flag = OMV_EMPTY;
  if (_omv_dc >= 1) flag |= OMV_DC;
  if (_omv_ff >= 1) flag |= OMV_FF_OUTPUT;
  if (_omv_input >= 1) flag |= OMV_FF_INPUT;
  if (_omv_test >= 1) flag |= OMV_TEST;
  // data preparing
  omv_calculate (stg, omv, flag);
  omv_sort (omv);

  // out file
  if (_omv_out_file_name != NULL)
  {
    if ((_omv_out_file = fopen (_omv_out_file_name, "w")) == NULL)
      Fatal("opening output file <%s>", _omv_out_file_name);
  }
  else
    _omv_out_file = stdout;
  //
  if (_omv_statistics > 0)
    omv_info_write (_omv_out_file, omv, flag);
  //
  omv_verilog_write (_omv_out_file, stg, omv, flag);
  fclose (_omv_out_file);

  // debug
  if (_debug_level > 0)
  {
    stg_debug (stderr, stg);
    omv_debug (stderr, omv);
  }

  omv_destroy (omv);
  stg_destroy (stg);

  Success;
}
