#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "oml_msgs.h"

const char *_oml_fmt_help =
  "%s: OptiMist tool for generation of Verilog netlist for DCs and FFs\n"
  "Usage: %s [FLAGS] [ELEMENT_NAME]\n"
  "FILE OPTIONS:\n"
  "  -o, --output OUT_FILE_NAME  output file [STDOUT]\n"
  "GENERAL OPTIONS:\n"
  "  -f, --format[N]             format of the output data (N=0,1,2,[3])\n"
  "      0 = check the validity of the element name\n"
  "      1 = pins description\n"
  "      2 = gate-level Verilog netlist\n"
  "      3 = transistor-level Verilog netlist\n"
  "INFORMATION OPTIONS:\n"
  "  -d, --debug[N]              level of debug information (N=[0],1,2,3,4)\n"
  "  -v, --version               print version and copyright\n"
  "  -h, --help                  print this help\n";

const char *_oml_fmt_version =
  "%s: OptiMist tool for generation of Verilog netlist for DCs and FFs\n"
  "Version: %d.%d\n"
  "Modified: %s\n"
  "Copyright (c) 2001, Danil Sokolov\n"
  "University of Newcastle, Newcastle upon Tyne, UK\n";

const int _oml_version_major = 1;
const int _oml_version_minor = 0;
const char *_oml_version_date = "5 December 2004";

char *oml_get_help(char *name)
{
  char *help = NULL;
  help = (char *) malloc (strlen (_oml_fmt_help) + 2 * strlen (name) + 1);
  sprintf (help, _oml_fmt_help, name, name);
  return help;
}

char *oml_get_version (char *name)
{
  char *version = NULL;
  version = (char *) malloc (strlen (name) + strlen (_oml_fmt_version) + strlen (_oml_version_date) + 1);
  (void) sprintf (version, _oml_fmt_version, name, _oml_version_major, _oml_version_minor, _oml_version_date);
  return version;
}
