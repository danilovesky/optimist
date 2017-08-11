#ifndef _OML_PARAMS_H_
#define _OML_PARAMS_H_

//=== DEFAULTS
#define _OML_FORMAT  4

//=== FILE OPTIONS
extern char *_oml_program_name;
extern char *_oml_in_file_name;
extern char *_oml_out_file_name;

extern FILE *_oml_in_file;
extern FILE *_oml_out_file;

//=== GENERAL OPTIONS
extern int _oml_format;

extern void oml_process_params(int argv, char **argc);

#endif
