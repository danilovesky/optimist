#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "param.h"
#include "omg_msgs.h"
#include "omg_params.h"

//=== FILE OPTIONS
char *_omg_program_name;
char *_omg_in_file_name;
char *_omg_out_file_name;

FILE *_omg_in_file;
FILE *_omg_out_file;

//=== GENERAL OPTIONS
int _omg_legend;
int _omg_connect;
int _omg_inputs;

void omg_process_params(int argc, char **argv)
{
  int i;
  int stdout_num = 1;
  int stdin_num  = 0;

  //=== FILE OPTIONS
  _omg_program_name = NULL;
  _omg_in_file_name = NULL;
  _omg_out_file_name = NULL;

  _omg_in_file = NULL;
  _omg_out_file = NULL;

  //=== GENERAL OPTIONS
  _omg_legend = _OMG_LEGEND;
  _omg_connect = _OMG_CONNECT;
  _omg_inputs = _OMG_INPUTS;

  _omg_program_name = strrchr (argv[0], '/');
  if (_omg_program_name == NULL) _omg_program_name = argv[0];
  else _omg_program_name++;

  if (argc < 2) i = 0;
  else i = 1;

  while (i > 0)
  {
    //=== FILE OPTIONS
    // _omg_in_file_name
    if (argv[i][0] != '-')
    {
      if (stdin_num == 0)
      {
        _omg_in_file_name = (char*) malloc (strlen(argv[i])+1);
        strcpy(_omg_in_file_name, argv[i]);
        stdin_num++;
        i++;
        if (i >= argc) i = 0;
        continue;
      }
      else
      {
        Fatal("more than one input file specified");
      }
    }
    else
    // -o _omg_out_file_name <=> --output _omg_out_file_name
    if (param_string (argc, argv, &i, "o", &_omg_out_file_name) || param_string (argc, argv, &i, "-output", &_omg_out_file_name))
    {
      if (strcmp (_omg_out_file_name, "-") == 0)
      {
        _omg_out_file_name = NULL;
        stdout_num--;
      }
    }
    else

    //=== GENERAL OPTIONS
    // -l <=> --legend
    if (param_bool (argc, argv, &i, "l") || param_bool (argc, argv, &i, "-legend"))
    {
      _omg_legend = 1;
    }
    else
    // -c <=> --connect
    if (param_bool (argc, argv, &i, "c") || param_bool (argc, argv, &i, "-connect"))
    {
      _omg_connect = 1;
    }
    else
    // -i <=> --inputs
    if (param_bool (argc, argv, &i, "i") || param_bool (argc, argv, &i, "-inputs"))
    {
      _omg_inputs = 1;
    }
    else

    //=== INFO OPTIONS
    // -h <=> --help
    if (param_bool (argc, argv, &i, "h") || param_bool (argc, argv, &i, "-help"))
    {
      if (i != 0)
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      fprintf(stderr, omg_get_help (_omg_program_name));
      Success;
    }
    else
    // -v <=> --version
    if (param_bool (argc, argv, &i, "v") || param_bool (argc, argv, &i, "-vesion"))
    {
      if (i != 0)
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      fprintf (stderr, omg_get_version (_omg_program_name));
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

