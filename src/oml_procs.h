#ifndef _OML_PROCS_H_
#define _OML_PROCS_H_

extern int oml_check (char *name);
extern int oml_pin_write (FILE *outfile, char *name);
extern int oml_gate_write (FILE *outfile, char *name);
extern int oml_transistor_write (FILE *outfile, char *name);

#endif
