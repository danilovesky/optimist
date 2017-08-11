#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "stg.h"
#include "stg_procs.h"
#include "omg_procs.h"
#include "omg_params.h"

int main (int argc, char **argv)
{
  stg_t *stg;

  stg = stg_create ();

  omg_process_params (argc, argv);

  // input file
  if (_omg_in_file_name != NULL)
  {
    if ((_omg_in_file = fopen (_omg_in_file_name, "r")) == NULL)
      Fatal("opening input file <%s>", _omg_in_file_name);
  }
  else
    _omg_in_file = stdin;
  stg_read (_omg_in_file, stg);
//  stg_compress(stg);
  fclose (_omg_in_file);

  // data preparing
  stg_sort (stg);

  // out file
  if (_omg_out_file_name != NULL)
  {
    if ((_omg_out_file = fopen (_omg_out_file_name, "w")) == NULL)
      Fatal("opening output file <%s>", _omg_out_file_name);
  }
  else
    _omg_out_file = stdout;
  dot_write (_omg_out_file, stg, _omg_legend, _omg_connect, _omg_inputs);
  fclose (_omg_out_file);

  stg_destroy (stg);

  Success;
}
