#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ome_msgs.h"

const char *_ome_fmt_help =
  "%s: OptiMist tool for exposure of outputs\n"
  "Usage: %s [FLAGS] [IN_FILE_NAME]\n"
  "FILE OPTIONS:\n"
  "  -o, --output OUT_FILE_NAME  output file [STDOUT]\n"
  "INFORMATION OPTIONS:\n"
  "  -d, --debug[N]              level of debug information (N=[0],1,2,3,4)\n"
  "  -v, --version               print version and copyright\n"
  "  -h, --help                  print this help\n";

const char *_ome_fmt_version =
  "%s: OptiMist tool for exposure of outputs\n"
  "Version: %d.%d\n"
  "Modified: %s\n"
  "Copyright (c) 2001, Danil Sokolov\n"
  "University of Newcastle, Newcastle upon Tyne, UK\n";

const int _ome_version_major = 1;
const int _ome_version_minor = 0;
const char *_ome_version_date = "5 December 2004";

char *ome_get_help(char *name)
{
  char *help = NULL;
  help = (char *) malloc (strlen (_ome_fmt_help) + 2 * strlen (name) + 1);
  sprintf (help, _ome_fmt_help, name, name);
  return help;
}

char *ome_get_version (char *name)
{
  char *version = NULL;
  version = (char *) malloc (strlen (name) + strlen (_ome_fmt_version) + strlen (_ome_version_date) + 1);
  (void) sprintf (version, _ome_fmt_version, name, _ome_version_major, _ome_version_minor, _ome_version_date);
  return version;
}
