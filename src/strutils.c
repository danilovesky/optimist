#include <stdlib.h>
#include <string.h>
#include "strutils.h"


/* === */
int indent_change (char* &indent_str, int indent_dir, char indent_char, int indent_delta)
{
  int indent_len = 0;
  if (indent_str != NULL) indent_len = strlen (indent_str);
  indent_len = indent_len + indent_dir*indent_delta;
  indent_str = (char *) realloc (indent_str, indent_len + 1);
  memset (indent_str, indent_char, indent_len);
  indent_str[indent_len] = '\0';
  return indent_len;
}
 
