#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "msgs.h"

void Error(char *format, ...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, "Error: ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
  exit(1);
}

void Warning (char *format, ...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, "Warning: ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}

void Message (char *format, ...)
{
  va_list args;
  va_start(args, format);
  fprintf(stderr, "- ");
  vfprintf(stderr, format, args);
  fprintf(stderr, "\n");
  va_end(args);
}

