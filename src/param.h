#ifndef _PARAM_H_
#define _PARAM_H_

extern int param_bool(int, char **, int *, char *);
extern int param_integer(int, char **, int *, char *, int *);
extern int param_float(int, char **, int *, char *, float *);
extern int param_string(int, char **, int *, char *, char **);

#endif
