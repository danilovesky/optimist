#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "omv_msgs.h"

const char *_omv_fmt_help =
  "%s: Optimist tool for mapping STG into Verilog netlist\n"
  "Usage: %s [FLAGS] [IN_FILE_NAME]\n"
  "FILE OPTIONS:\n"
  "  -o, --output OUT_FILE_NAME  output file [STDOUT]\n"
  "GENERAL OPTIONS:\n"
  "  -t, --test                  generate circuit ready for off-line testing\n"
  "  -s, --statistics            include statistic comments into Verilog netlist\n"
  "  -dc,--david_cell            provide detail DC statistics\n"
  "  -ff,--flip_flop             provide detail FF statistics\n"
  "  -i, --input                 provide input signals statistics\n"
  "INFORMATION OPTIONS:\n"
  "  -d, --debug[N]              level of debug information (N=[0],1,2,3,4)\n"
  "  -v, --version               print version and copyright\n"
  "  -h, --help                  print this help\n";

const char *_omv_fmt_version =
  "%s: Optimist tool for mapping STG into Verilog netlist\n"
  "Version: %d.%d\n"
  "Modified: %s\n"
  "Copyright (c) 2001, Danil Sokolov\n"
  "University of Newcastle, Newcastle upon Tyne, UK\n";

const int _omv_version_major = 1;
const int _omv_version_minor = 0;
const char *_omv_version_date = "5 December 2004";

char *omv_get_help(char *name)
{
  char *help = NULL;
  help = (char *) malloc (strlen (_omv_fmt_help) + 2 * strlen (name) + 1);
  sprintf (help, _omv_fmt_help, name, name);
  return help;
}

char *omv_get_version (char *name)
{
  char *version = NULL;
  version = (char *) malloc (strlen (name) + strlen (_omv_fmt_version) + strlen (_omv_version_date) + 1);
  (void) sprintf (version, _omv_fmt_version, name, _omv_version_major, _omv_version_minor, _omv_version_date);
  return version;
}
