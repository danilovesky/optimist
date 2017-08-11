#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "param.h"
#include "omt_msgs.h"
#include "omt_params.h"

//=== FILE OPTIONS
char *_omt_program_name;
char *_omt_in_file_name;
char *_omt_out_file_name;

FILE *_omt_in_file;
FILE *_omt_out_file;

//=== GENERAL OPTIONS
int _omt_level;
int _omt_fanin;

void omt_process_params(int argc, char **argv)
{
  int i;
  int stdout_num = 1;
  int stdin_num = 0;

  //=== FILE OPTIONS
  _omt_program_name = NULL;
  _omt_in_file_name = NULL;
  _omt_out_file_name = NULL;

  _omt_in_file = NULL;
  _omt_out_file = NULL;

  //=== GENERAL OPTIONS
  _omt_level = _OMT_LEVEL;
  _omt_fanin = _OMT_FANIN;

  _omt_program_name = strrchr (argv[0], '/');
  if (_omt_program_name == NULL) _omt_program_name = argv[0];
  else _omt_program_name++;

  if (argc < 2) i = 0;
  else i = 1;

  while (i > 0)
  {
    //=== FILE OPTIONS
    // _omt_in_file_name
    if (argv[i][0] != '-')
    {
      if (stdin_num == 0)
      {
        _omt_in_file_name = (char*) malloc (strlen(argv[i])+1);
        strcpy(_omt_in_file_name, argv[i]);
        stdin_num++;
        i++;
        if (i >= argc) i = 0;
        continue;
      }
      else
        Fatal("more than one input file specified");
    }
    else
    // -o _omt_out_file_name <=> --output _omt_out_file_name
    if (param_string (argc, argv, &i, "o", &_omt_out_file_name) || param_string (argc, argv, &i, "-output", &_omt_out_file_name))
    {
      if (strcmp (_omt_out_file_name, "-") == 0)
      {
        _omt_out_file_name = NULL;
        stdout_num--;
      }
    }
    else

    //=== GENERAL OPTIONS
    // -l[n] <=> --level[n]
    if (param_integer (argc, argv, &i, "l", &_omt_level) || param_integer (argc, argv, &i, "-level", &_omt_level))
    {
      if (_omt_level < 0) _omt_level = 0;
    }
    else

    // -j[n] <=> --join-fanin[n]
    if (param_integer (argc, argv, &i, "j", &_omt_fanin) || param_integer (argc, argv, &i, "-join-fanin", &_omt_fanin))
    {
      if (_omt_fanin < 0) _omt_fanin = 0;
    }
    else

    //=== INFO OPTIONS
    // -h <=> --help
    if (param_bool (argc, argv, &i, "h") || param_bool (argc, argv, &i, "-help"))
    {
      if (i != 0)
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      fprintf(stderr, omt_get_help (_omt_program_name));
      Success;
    }
    else
    // -v <=> --version
    if (param_bool (argc, argv, &i, "v") || param_bool (argc, argv, &i, "-vesion"))
    {
      if (i != 0)
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      fprintf (stderr, omt_get_version (_omt_program_name));
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

