#ifndef _OME_PARAMS_H_
#define _OME_PARAMS_H_

#include <stdio.h>
//=== DEFAULTS
#define _OME_LEVEL  0

//=== FILE OPTIONS
extern char *_ome_program_name;
extern char *_ome_in_file_name;
extern char *_ome_out_file_name;

extern FILE *_ome_in_file;
extern FILE *_ome_out_file;

//=== GENERAL OPTIONS
extern void ome_process_params(int argv, char **argc);

#endif
