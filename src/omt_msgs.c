#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "omt_msgs.h"

const char *_omt_fmt_help =
  "%s: OptiMist tool for elimination of redundant places\n"
  "Usage: %s [FLAGS] [IN_FILE_NAME]\n"
  "FILE OPTIONS:\n"
  "  -o, --output OUT_FILE_NAME  output file [STDOUT]\n"
  "GENERAL OPTIONS:\n"
  "  -l, --level[N]              level of transformation (N=0,1,2,3,4,[5])\n"
  "      0 = remove transient arcs only\n"
  "      1 = 0 + move initial marking to mandatory places\n"
  "      2 = 1 + recalculate context signals\n"
  "      3 = 2 + eliminate redundant places\n"
  "      4 = 3 + remove transient arcs, not connected places and transitions\n"
  "      5 = 4 + simplify join transitions\n"
  "  -j, --join-fanin[N]         max join transitions fanin (N=0,1,[2],3)\n"
  "      0 = no control on the join transitions fanin\n"
  "    1-3 = restrict join transitions fanin to the given number,\n"
  "          higher value for this parameter is not practical\n"
  "INFORMATION OPTIONS:\n"
  "  -d, --debug[N]              level of debug information (N=[0],1,2,3,4)\n"
  "  -v, --version               print version and copyright\n"
  "  -h, --help                  print this help\n";

const char *_omt_fmt_version =
  "%s: OptiMist tool for elimination of redundant places\n"
  "Version: %d.%d\n"
  "Modified: %s\n"
  "Copyright (c) 2001, Danil Sokolov\n"
  "University of Newcastle, Newcastle upon Tyne, UK\n";

const int _omt_version_major = 1;
const int _omt_version_minor = 0;
const char *_omt_version_date = "05 December 2004";

char *omt_get_help(char *name)
{
  char *help = NULL;
  help = (char *) malloc (strlen (_omt_fmt_help) + 2 * strlen (name) + 1);
  sprintf (help, _omt_fmt_help, name, name);
  return help;
}

char *omt_get_version (char *name)
{
  char *version = NULL;
  version = (char *) malloc (strlen (name) + strlen (_omt_fmt_version) + strlen (_omt_version_date) + 1);
  (void) sprintf (version, _omt_fmt_version, name, _omt_version_major, _omt_version_minor, _omt_version_date);
  return version;
}

