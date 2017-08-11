#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "param.h"
#include "omv_msgs.h"
#include "omv_params.h"

//=== FILE OPTIONS
char *_omv_program_name;
char *_omv_in_file_name;
char *_omv_out_file_name;

FILE *_omv_in_file;
FILE *_omv_out_file;

//=== GENERAL OPTIONS
int _omv_test;
int _omv_statistics;
int _omv_dc;
int _omv_ff;
int _omv_input;

void omv_process_params(int argc, char **argv)
{
  int i;
  int stdout_num = 1;
  int stdin_num = 0;

  //=== FILE OPTIONS
  _omv_program_name = NULL;
  _omv_in_file_name = NULL;
  _omv_out_file_name = NULL;

  _omv_in_file = NULL;
  _omv_out_file = NULL;

  //=== GENERAL OPTIONS
  _omv_test = _OMV_TEST;
  _omv_statistics = _OMV_STATISTICS;
  _omv_dc = _OMV_DC;
  _omv_ff = _OMV_FF;
  _omv_input = _OMV_INPUT;

  _omv_program_name = strrchr (argv[0], '/');
  if (_omv_program_name == NULL) _omv_program_name = argv[0];
  else _omv_program_name++;

  if (argc < 2) i = 0;
  else i = 1;

  while (i > 0)
  {
    //=== FILE OPTIONS
    // _omv_in_file_name
    if (argv[i][0] != '-')
    {
      if (stdin_num == 0)
      {
        _omv_in_file_name = (char*) malloc (strlen(argv[i])+1);
        strcpy(_omv_in_file_name, argv[i]);
        stdin_num++;
        i++;
        if (i >= argc) i = 0;
        continue;
      }
      else
        Fatal("more than one input file specified");
    }
    else
    // -o _omv_out_file_name <=> --output _omv_out_file_name
    if (param_string (argc, argv, &i, "o", &_omv_out_file_name) || param_string (argc, argv, &i, "-output", &_omv_out_file_name))
    {
      if (strcmp (_omv_out_file_name, "-") == 0)
      {
        _omv_out_file_name = NULL;
        stdout_num--;
      }
    }
    else

    //=== GENERAL OPTIONS
    // -t <=> --test
    if (param_bool (argc, argv, &i, "t") || param_bool (argc, argv, &i, "-test"))
    {
      _omv_test = 1;
    }
    else
    // -s <=> --statistics
    if (param_bool (argc, argv, &i, "s") || param_bool (argc, argv, &i, "-statistics"))
    {
      _omv_statistics = 1;
    }
    else
    // -dc <=> --david_cell
    if (param_bool (argc, argv, &i, "dc") || param_bool (argc, argv, &i, "-david_cell"))
    {
      _omv_dc = 1;
    }
    else
    // -ff <=> --flip_flop
    if (param_bool (argc, argv, &i, "ff") || param_bool (argc, argv, &i, "-flip_flop"))
    {
      _omv_ff = 1;
    }
    else
    // -i <=> --input
    if (param_bool (argc, argv, &i, "i") || param_bool (argc, argv, &i, "-input"))
    {
      _omv_input = 1;
    }
    else

    //=== INFO OPTIONS
    // -h <=> --help
    if (param_bool (argc, argv, &i, "h") || param_bool (argc, argv, &i, "-help"))
    {
      if (i != 0)
      {
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      }
      fprintf(stderr, omv_get_help (_omv_program_name));
      Success;
    }
    else
    // -v <=> --version
    if (param_bool (argc, argv, &i, "v") || param_bool (argc, argv, &i, "-vesion"))
    {
      if (i != 0)
      {
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      }
      fprintf (stderr, omv_get_version (_omv_program_name));
      Success;
    }
    else

    // DEBUG OPTIONS
    // -d[n] <=> --debug[n]
    if (param_integer (argc, argv, &i, "d", &_debug_level) || param_integer (argc, argv, &i, "-debug", &_debug_level))
    {
      if (_debug_level < 0) _debug_level = 0;
    }
    // UNRECOGNIZED FLAG
    else
    {
      Fatal("%s unrecognized flag", argv[i]);
    }
  }

  if (stdout_num > 1)
    Fatal("More than one file redirected to standard output");
}

