#ifndef _OMV_PARAMS_H_
#define _OMV_PARAMS_H_

//=== DEFAULTS
#define _OMV_TEST         0
#define _OMV_STATISTICS   0
#define _OMV_DC           0
#define _OMV_FF           0
#define _OMV_INPUT        0

//=== FILE OPTIONS
extern char *_omv_program_name;
extern char *_omv_in_file_name;
extern char *_omv_out_file_name;

extern FILE *_omv_in_file;
extern FILE *_omv_out_file;

//=== GENERAL OPTIONS
extern int _omv_test;
extern int _omv_statistics;
extern int _omv_dc;
extern int _omv_ff;
extern int _omv_input;

extern void omv_process_params(int argv, char **argc);

#endif
