#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "stg.h"
#include "stg_procs.h"
#include "ome_params.h"
#include "ome_procs.h"

int main (int argc, char **argv)
{
  stg_t *stg;

  stg = stg_create ();

  ome_process_params (argc, argv);

  // input file
  if (_ome_in_file_name != NULL)
  {
    if ((_ome_in_file = fopen (_ome_in_file_name, "r")) == NULL)
      Fatal ("opening input file <%s>", _ome_in_file_name);
  }
  else
    _ome_in_file = stdin;
  stg_read (_ome_in_file, stg);
  fclose (_ome_in_file);
  stg_compress(stg);

  // === general options ===
  ome_output_exposition (stg);
  // data preparing
  stg_sort (stg);

  // out file
  if (_ome_out_file_name != NULL)
  {
    if ((_ome_out_file = fopen (_ome_out_file_name, "w")) == NULL)
      Fatal("opening output file <%s>", _ome_out_file_name);
  }
  else
    _ome_out_file = stdout;
  stg_write (_ome_out_file, stg);
  fclose (_ome_out_file);

  // debug
  if (_debug_level > 0)
    stg_debug (stderr, stg);

  stg_destroy (stg);

  Success;
}

