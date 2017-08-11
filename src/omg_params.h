#ifndef _OMG_PARAMS_H_
#define _OMG_PARAMS_H_

//=== DEFAULTS
#define _OMG_LEGEND         0
#define _OMG_CONNECT        0
#define _OMG_INPUTS         0

//=== FILE OPTIONS
extern char *_omg_program_name;
extern char *_omg_in_file_name;
extern char *_omg_out_file_name;

extern FILE *_omg_in_file;
extern FILE *_omg_out_file;

//=== GENERAL OPTIONS
extern int _omg_legend;
extern int _omg_connect;
extern int _omg_inputs;

extern void omg_process_params(int argv, char **argc);

#endif
