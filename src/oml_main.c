#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "oml_params.h"
#include "oml_procs.h"

int main (int argc, char **argv)
{
  oml_process_params (argc, argv);
  if (_oml_out_file_name != NULL)
  {
    if ((_oml_out_file = fopen (_oml_out_file_name, "w")) == NULL)
      Fatal("opening output file <%s>", _oml_out_file_name);
  }
  else
    _oml_out_file = stdout;
  switch ( oml_check(_oml_in_file_name) )
  {
    case 0:
      if (_oml_format == 1)
        oml_pin_write (_oml_out_file, _oml_in_file_name);
      else
      if (_oml_format == 2)
        oml_gate_write (_oml_out_file, _oml_in_file_name);
      else
      if (_oml_format == 3)
        oml_transistor_write (_oml_out_file, _oml_in_file_name);
      else
        Message ("Element name is correct");
      fclose (_oml_out_file);
      break;
    case 1:
      Message ("Incorrect element name format: error in element type");
      break;
    case 2:
      Message ("Incorrect element name format: error in request/set logic");
      break;
    case 3:
      Message ("Incorrect element name format: error in acknowledgement/reset logic");
      break;
    default:
      Message ("Incorrect element name format: cannot give more detail");
  }
  Success;
}
