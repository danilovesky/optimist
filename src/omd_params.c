#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "param.h"
#include "omd_msgs.h"
#include "omd_params.h"

//=== FILE OPTIONS
char *_omd_program_name;
char *_omd_in_file_name;
char *_omd_out_file_name;

FILE *_omd_in_file;
FILE *_omd_out_file;

//=== GENERAL OPTIONS
int _omd_level;
int _omd_num_loop;
int _omd_start;
int _omd_end;
int _omd_fork;
int _omd_join;
int _omd_choice;
int _omd_merge;
int _omd_token;
char *_omd_redundant_names;
char *_omd_separator_names;


void omd_process_params(int argc, char **argv)
{
  int i;
  int stdout_num = 1;
  int stdin_num = 0;

  //=== FILE OPTIONS
  _omd_program_name = NULL;
  _omd_in_file_name = NULL;
  _omd_out_file_name = NULL;

  _omd_in_file = NULL;
  _omd_out_file = NULL;

  //=== GENERAL OPTIONS
  _omd_level = _OMD_LEVEL;
  _omd_num_loop = _OMD_NUM_LOOP;
  _omd_start = _OMD_START;
  _omd_end = _OMD_END;
  _omd_fork = _OMD_FORK;
  _omd_join = _OMD_JOIN;
  _omd_choice = _OMD_CHOICE;
  _omd_merge = _OMD_MERGE;
  _omd_token = _OMD_TOKEN;
  _omd_redundant_names = NULL;
  _omd_separator_names = NULL;

  _omd_program_name = strrchr (argv[0], '/');
  if (_omd_program_name == NULL) _omd_program_name = argv[0];
  else _omd_program_name++;

  if (argc < 2) i = 0;
  else i = 1;

  while (i > 0)
  {
    //=== FILE OPTIONS
    // _omd_in_file_name
    if (argv[i][0] != '-')
    {
      if (stdin_num == 0)
      {
        _omd_in_file_name = (char*) malloc (strlen(argv[i])+1);
        strcpy(_omd_in_file_name, argv[i]);
        stdin_num++;
        i++;
        if (i >= argc) i = 0;
        continue;
      }
      else
        Fatal("more than one input file specified");
    }
    else
    // -o _omd_out_file_name <=> --output _omd_out_file_name
    if (param_string (argc, argv, &i, "o", &_omd_out_file_name) || param_string (argc, argv, &i, "-output", &_omd_out_file_name))
    {
      if (strcmp (_omd_out_file_name, "-") == 0)
      {
        free (_omd_out_file_name);
        _omd_out_file_name = NULL;
        stdout_num--;
      }
    }
    else

    //=== GENERAL OPTIONS
    // -l[n] <=> --level[n]
    if (param_integer (argc, argv, &i, "l", &_omd_level) || param_integer (argc, argv, &i, "-level", &_omd_level))
    {
      if (_omd_level < 0) _omd_level = 0;
    }
    else
    // -n[n] <=> --num-loop[n]
    if (param_integer (argc, argv, &i, "n", &_omd_num_loop) || param_integer (argc, argv, &i, "-num-loop", &_omd_num_loop))
    {
      if (_omd_num_loop < 0) _omd_num_loop = _OMD_NUM_LOOP;
    }
    else
    // -f <=> --fork
    if (param_bool (argc, argv, &i, "f") || param_bool (argc, argv, &i, "-fork"))
    {
      _omd_fork = 1;
    }
    else
    // -j <=> --join
    if (param_bool (argc, argv, &i, "j") || param_bool (argc, argv, &i, "-join"))
    {
      _omd_join = 1;
    }
    else
    // -c <=> --choice
    if (param_bool (argc, argv, &i, "c") || param_bool (argc, argv, &i, "-choice"))
    {
      _omd_choice = 1;
    }
    else
    // -m <=> --merge
    if (param_bool (argc, argv, &i, "m") || param_bool (argc, argv, &i, "-merge"))
    {
      _omd_merge = 1;
    }
    else
    // -t <=> --token
    if (param_bool (argc, argv, &i, "t") || param_bool (argc, argv, &i, "-token"))
    {
      _omd_token = 1;
    }
    else
    // -r place[,place...] <=> --redundant place[,place...]
    if (param_string (argc, argv, &i, "r", &_omd_redundant_names) || param_string (argc, argv, &i, "-redundant", &_omd_redundant_names))
    {
      if (strcmp (_omd_redundant_names, "-") == 0)
      {
        free (_omd_redundant_names);
        _omd_redundant_names = NULL;
      }
    }
    else
    // -s place[,place...] <=> --separator place[,place...]
    if (param_string (argc, argv, &i, "s", &_omd_separator_names) || param_string (argc, argv, &i, "-separator", &_omd_separator_names))
    {
      if (strcmp (_omd_separator_names, "-") == 0)
      {
        free (_omd_separator_names);
        _omd_separator_names = NULL;
      }
    }
    else

    //=== INFO OPTIONS
    // -h <=> --help
    if (param_bool (argc, argv, &i, "h") || param_bool (argc, argv, &i, "-help"))
    {
      if (i != 0)
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      fprintf(stderr, omd_get_help (_omd_program_name));
      Success;
    }
    else
    // -v <=> --version
    if (param_bool (argc, argv, &i, "v") || param_bool (argc, argv, &i, "-vesion"))
    {
      if (i != 0)
        Warning ("\"%s\" must be the only flag in the command. All other flags ignored.",argv[i-1]);
      fprintf (stderr, omd_get_version (_omd_program_name));
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

