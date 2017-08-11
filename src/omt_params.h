#ifndef _OMT_PARAMS_H_
#define _OMT_PARAMS_H_

#define _OMT_LEVEL     5
#define _OMT_FANIN     2

//=== FILE OPTIONS
extern char *_omt_program_name;
extern char *_omt_in_file_name;
extern char *_omt_out_file_name;

extern FILE *_omt_in_file;
extern FILE *_omt_out_file;

//=== GENERAL OPTIONS
extern int _omt_level;
extern int _omt_fanin;

extern void omt_process_params(int argv, char **argc);

#endif
