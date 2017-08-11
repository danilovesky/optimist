#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "omd_msgs.h"

const char *_omd_fmt_help =
  "%s: OptiMist tool for detection of redundant places\n"
  "Usage: %s [FLAGS] [IN_FILE_NAME]\n"
  "FILE OPTIONS:\n"
  "  -o, --output OUT_FILE_NAME  output file [STDOUT]\n"
  "GENERAL OPTIONS:\n"
  "  -l, --level[N]              level of redundant places detection (N=0,1,2,[3])\n"
  "      0 = all places are tagged as non-redundant\n"
  "      1 = 0 + heuristic A is used to detect redundant places\n"
  "      2 = 1 + heuristics B is applied to detect redundant places\n"
  "      3 = 2 + heuristic C is applied to detect redundant places\n"
  "      Heuristic A (latency reduction): Places after input transitions but\n"
  "              before output transitions are tagged as redundant.\n"
  "      Heuristic B (size reduction): The chains of places between redundant\n"
  "              places which are detected by heuristic A are considered. The\n"
  "              places of each chain are tagged starting from the beginning of\n"
  "              of the chain and going in the direction of consuming-producing\n"
  "              arcs. The last place in each chain is skipped.\n"
  "      Heuristic C (size reduction): All non-tagged places (which are the\n"
  "              last places in the chains) are tagged individually.\n"
  "  -r, --redundant PLACE_LIST  comma-separated list of redundant places []\n"
  "  -s, --separator PLACE_LIST  comma-separated list of mandatory places []\n"
  "  -n, --num-loop[N]           minimum number of DC in a loop (N=1,2,[3])\n"
  "  -f, --fork                  consider places after fork as mandatory\n"
  "  -j, --join                  consider places after join as mandatory\n"
  "  -c, --choice                consider choice places as mandatory\n"
  "  -m, --merge                 consider merge places as mandatory\n"
  "  -t, --token                 consider places marked with a token as mandatory\n"
  "INFORMATION OPTIONS:\n"
  "  -d, --debug[N]              level of debug information (N=[0],1,2,3,4)\n"
  "  -v, --version               print version and copyright\n"
  "  -h, --help                  print this help\n";

const char *_omd_fmt_version =
  "%s: OptiMist tool for detection of redundant places\n"
  "Version: %d.%d\n"
  "Modified: %s\n"
  "Copyright (c) 2001, Danil Sokolov\n"
  "University of Newcastle, Newcastle upon Tyne, UK\n";

const int _omd_version_major = 1;
const int _omd_version_minor = 0;
const char *_omd_version_date = "5 December 2004";

char *omd_get_help(char *name)
{
  char *help = NULL;
  help = (char *) malloc (strlen (_omd_fmt_help) + 2 * strlen (name) + 1);
  sprintf (help, _omd_fmt_help, name, name);
  return help;
}

char *omd_get_version (char *name)
{
  char *version = NULL;
  version = (char *) malloc (strlen (name) + strlen (_omd_fmt_version) + strlen (_omd_version_date) + 1);
  (void) sprintf (version, _omd_fmt_version, name, _omd_version_major, _omd_version_minor, _omd_version_date);
  return version;
}
