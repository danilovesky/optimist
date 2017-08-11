#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "omg_msgs.h"

const char *_omg_fmt_help =
  "%s: OptiMist tool for drawing STG using GraphVis DOT format\n"
  "Usage: %s [FLAGS] [IN_FILE_NAME]\n"
  "FILE OPTIONS:\n"
  "  -o, --output OUT_FILE_NAME  output file [STDOUT]\n"
  "GENERAL OPTIONS:\n"
  "  -l, --legend                show legend\n"
  "  -c, --connect               connect read-arcs to places\n"
  "  -i, --inputs                show elementary cycles for inputs\n"
  "INFORMATION OPTIONS:\n"
  "  -d, --debug[N]              level of debug information (N=[0],1,2,3,4)\n"
  "  -v, --version               print version and copyright\n"
  "  -h, --help                  print this help\n";

const char *_omg_fmt_version =
  "%s: OptiMist tool for drawing STG using GraphVis DOT format\n"
  "Version: %d.%d\n"
  "Modified: %s\n"
  "Copyright (c) 2001, Danil Sokolov\n"
  "University of Newcastle, Newcastle upon Tyne, UK\n";

const int _omg_version_major = 1;
const int _omg_version_minor = 0;
const char *_omg_version_date = "5 December 2004";

char *omg_get_help(char *name)
{
  char *help = NULL;
  help = (char *) malloc (strlen (_omg_fmt_help) + 2 * strlen (name) + 1);
  sprintf (help, _omg_fmt_help, name, name);
  return help;
}

char *omg_get_version (char *name)
{
  char *version = NULL;
  version = (char *) malloc (strlen (name) + strlen (_omg_fmt_version) + strlen (_omg_version_date) + 1);
  (void) sprintf (version, _omg_fmt_version, name, _omg_version_major, _omg_version_minor, _omg_version_date);
  return version;
}

