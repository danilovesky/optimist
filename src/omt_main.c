#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "stg.h"
#include "stg_procs.h"
#include "omt_params.h"
#include "omt_procs.h"

int main (int argc, char **argv)
{
  stg_t *stg;

  stg = stg_create ();

  omt_process_params (argc, argv);

  // input file
  if (_omt_in_file_name != NULL)
  {
    if ((_omt_in_file = fopen (_omt_in_file_name, "r")) == NULL)
      Fatal("opening input file <%s>", _omt_in_file_name);
  }
  else
    _omt_in_file = stdin;
  stg_read (_omt_in_file, stg);
  fclose (_omt_in_file);
  stg_compress (stg);

  // === general options ===
  if (_omt_level >= 0)
  {
// Do we really need to mark all palses after fork as mandatory if one of such places is madatory?
//    omt_fanin (stg);
    if (_omt_level >= 1)
    {
      omt_marking (stg);
      if (_omt_level >= 2)
      {
        omt_context (stg);
        if (_omt_level >= 3)
        {
          omt_redundant (stg);
          if (_omt_level >= 4)
          {
            omt_compress (stg);
            if (_omt_level >= 5)
            {
              omt_simplify (stg, _omt_fanin);
            }
          }
        }
      }
    }
  }

  // data preparing
  stg_sort (stg);

  // out file
  if (_omt_out_file_name != NULL)
  {
    if ((_omt_out_file = fopen (_omt_out_file_name, "w")) == NULL)
      Fatal("opening output file <%s>", _omt_out_file_name);
  }
  else
    _omt_out_file = stdout;
  stg_write (_omt_out_file, stg);
  fclose (_omt_out_file);

  // debug
//  if (_debug_level > 0)
//    stg_debug (stderr, stg);

  stg_destroy (stg);
  Success;
}






