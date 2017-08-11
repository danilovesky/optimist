#ifndef _STRUTILS_H_
#define _STRUTILS_H_

#define INDENT_CHR            ' '
#define INDENT_DELTA          2

extern int indent_change (char* &indent_str, int indent_dir, char indent_char = INDENT_CHR, int indent_delta = INDENT_DELTA);

#endif

 
