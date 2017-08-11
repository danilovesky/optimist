#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "param.h"
#include "oml_msgs.h"
#include "oml_params.h"

//=== FILE OPTIONS
char *_oml_program_name;
char *_oml_in_file_name;
char *_oml_out_file_name;

FILE *_oml_in_file;
FILE *_oml_out_file;

//=== GENERAL OPTIONS
int _oml_format;

void oml_process_params(int argc, char **argv)
{
  int i;
  int stdout_num = 1;
  int stdin_num = 0;

  //=== FILE OPTIONS
  _oml_program_name = NULL;
  _oml_in_file_name = NULL;
  _oml_out_file_name = NULL;

  _oml_in_file = NULL;
  _oml_out_file = NULL;

  //=== GENERAL OPTIONS
  _oml_format = _OML_FORMAT;

  _oml_program_name = strrchr (argv[0], '/');
  if (_oml_program_name == NULL) _oml_program_name = argv[0];
  else _oml_program_name++;

  if (argc < 2) i = 0;
  else i = 1;

  while (i > 0)
  {
    //=== FILE OPTIONS
    // _oml_in_file_name
    if (argv[i][0] != '-')
    {
      if (stdin_num == 0)
      {
        _oml_in_file_name = (char*) malloc (strlen(argv[i])+1);
        strcpy(_oml_in_file_name, argv[i]);
        stdin_num++;
        i++;
        if (i >= argc) i = 0;
        continue;
      }
      else
        Fatal("more than one input file specified");
    }
    else
    // -o _oml_out_file_name <=> --output _oml_out_file_name
    if (param_string (argc, argv, &i, "o", &_oml_out_file_name) || param_string (argc, argv, &i, "-output", &_oml_out_file_name))
    {
      if (strcmp (_oml_out_file_name, "-") == 0)
      {
        _oml_out_file_name = NULL;
        stdout_num--;
      }
    }
    else

    //=== GENERAL OPTIONS
    // -f[n] <=> --format[n]
    if (param_integer (argc, argv, &i, "f", &_oml_format) || param_integer (argc, argv, &i, "-format", &_oml_format))
    {
      if (_oml_format < 0) _oml_format = 0;
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
      fprintf(stderr, oml_get_help (_oml_program_name));
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
      fprintf (stderr, oml_get_version (_oml_program_name));
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

