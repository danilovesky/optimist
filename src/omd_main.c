#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "stg.h"
#include "stg_procs.h"
#include "omd_params.h"
#include "omd_procs.h"

int main (int argc, char **argv)
{
  stg_t *stg;

  stg = stg_create ();

  omd_process_params (argc, argv);

  // input file
  if (_omd_in_file_name != NULL)
  {
    if ((_omd_in_file = fopen (_omd_in_file_name, "r")) == NULL)
      Fatal("opening input file <%s>", _omd_in_file_name);
  }
  else
    _omd_in_file = stdin;
  stg_read (_omd_in_file, stg);
  fclose (_omd_in_file);
  stg_compress (stg);

  // === general options ===
  // level
  if (_omd_level >= 0)
  {
    omd_mandatory (stg);
    omd_force_class (stg, _omd_start, _omd_end, _omd_fork, _omd_join, _omd_choice, _omd_merge, _omd_token);
    omd_force_list (stg, _omd_redundant_names, _omd_separator_names, _omd_num_loop);
    if (_omd_level >= 1)
    {
      omd_eliminate_input_output (stg, _omd_num_loop);
      if (_omd_level >= 2)
      {
        omd_eliminate_chain (stg, _omd_num_loop);
        if (_omd_level >= 3)
        {
          omd_eliminate_one (stg, _omd_num_loop);
        }
      }
    }
  }

  // data preparing
  stg_sort (stg);

  // out file
  if (_omd_out_file_name != NULL)
  {
    if ((_omd_out_file = fopen (_omd_out_file_name, "w")) == NULL)
        Fatal("opening output file <%s>", _omd_out_file_name);
  }
  else
    _omd_out_file = stdout;
  stg_write (_omd_out_file, stg);
  fclose (_omd_out_file);

  // debug
  if (_debug_level > 0)
    stg_debug (stderr, stg);

  stg_destroy (stg);

  Success;
}

