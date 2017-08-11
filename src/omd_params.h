#ifndef _OMD_PARAMS_H_
#define _OMD_PARAMS_H_

//=== DEFAULTS
#define _OMD_LEVEL           3
#define _OMD_NUM_LOOP        3
#define _OMD_START           1
#define _OMD_END             1
#define _OMD_FORK            0
#define _OMD_JOIN            0
#define _OMD_CHOICE          0
#define _OMD_MERGE           0
#define _OMD_TOKEN           0

//=== FILE OPTIONS
extern char *_omd_program_name;
extern char *_omd_in_file_name;
extern char *_omd_out_file_name;

extern FILE *_omd_in_file;
extern FILE *_omd_out_file;

//=== GENERAL OPTIONS
extern int _omd_level;
extern int _omd_num_loop;
extern int _omd_start;
extern int _omd_end;
extern int _omd_fork;
extern int _omd_join;
extern int _omd_choice;
extern int _omd_merge;
extern int _omd_token;
extern char *_omd_redundant_names;
extern char *_omd_separator_names;


extern void omd_process_params(int argv, char **argc);

#endif
