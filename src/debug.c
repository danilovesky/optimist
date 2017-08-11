#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

int _debug_level = 0;
int _warnings_show = 1;

void Error (char *format, ...)
{
  va_list args;
  va_start (args, format);
  fprintf (stderr, "Error: ");
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);
}

void Fatal (char *format, ...)
{
  va_list args;
  va_start (args, format);
  fprintf (stderr, "Error: ");
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);
  Failure;
}

void Warning (char *format, ...)
{
  va_list args;
  if (!_warnings_show) return;
  va_start (args, format);
  fprintf (stderr, "Warning: ");
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);
}

void Message (char *format, ...)
{
  va_list args;
  va_start (args, format);
  fprintf (stderr, "- ");
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);
}

void Verbose (int level, char *format, ...)
{
  va_list args;
  if (level > _debug_level) return;
  va_start (args, format);
  fprintf (stderr, "|> ");
  for (; level > 0; level--) fprintf (stderr,"  ");
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);
}

