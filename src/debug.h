#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define Failure exit(EXIT_FAILURE)
#define Success exit(EXIT_SUCCESS)

#define DEBUG_GLOBAL      1
#define DEBUG_STACK       2
#define DEBUG_PROC        3
#define DEBUG_LOCAL       4

extern int _debug_level;
extern int _warnings_show;

extern void Message (char *format, ...);
extern void Warning (char *format, ...);
extern void Verbose (int level, char *format, ...);
extern void Error (char *format, ...);
extern void Fatal (char *format, ...);

#endif
