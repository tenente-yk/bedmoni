#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "debug.h"
#if defined (QT_CORE_LIB)
#include <QDebug>
#undef DEBUG_WINDOW
#endif

#if defined (UNIX)
#define _vsnprintf vsnprintf
#endif

#define DEBUG       1

#if defined (DEBUG_WINDOW) && defined(WIN32)
static HANDLE ystderr, ystdout, ystdin;
#endif
#if defined(DEBUG_FILE)
static FILE * flog = NULL;
#endif

static void yfprintf(FILE *stream, char *fmt, ...)
{
#if defined (DEBUG_WINDOW) && defined(WIN32)
  char s[255];
  unsigned long v;
#endif
  va_list  args;
  va_start(args, fmt);
#if defined (UNIX)
  vfprintf(stream, fmt, args);
#endif
#if defined (QT_CORE_LIB)
  char s[255];
  vsprintf(s, fmt, args);
  if (s[strlen(s)-1] == '\n') s[strlen(s)-1] = 0;
  if (s[strlen(s)-1] == '\r') s[strlen(s)-1] = 0;
  qDebug() << s;
#endif
#if STDIO_LOGGING
  if (stream == stdout && flogout)
  {
    vfprintf(flogout, fmt, args);
    fflush(flogout);
  }
  if (stream == stderr && flogerr)
  {
    vfprintf(flogerr, fmt, args);
    fflush(flogerr);
  }
#endif
#if defined (DEBUG_WINDOW) && defined(WIN32)
  if (stream == stdout)
  {
    vsprintf(s, fmt, args);
    WriteConsole(ystdout, s, strlen(s), &v, NULL);
  }
  if (stream == stderr)
  {
    vsprintf(s, fmt, args);
    WriteConsole(ystderr, s, strlen(s), &v, NULL);
  }
#endif
  va_end( args );
}

void debug_init(void)
{
#if defined (DEBUG_WINDOW) && defined(WIN32)
  AllocConsole();
  ystdin = GetStdHandle(STD_INPUT_HANDLE);
  ystdout = GetStdHandle(STD_ERROR_HANDLE);
  ystderr = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

#if defined(DEBUG_FILE)
  flog = fopen("debug.log", "wt");
#endif

 // debug("debug_init okay.\n");
}

void debug_deinit(void)
{
#if defined (DEBUG_WINDOW) && defined(WIN32)
  FreeConsole();
#endif
#if defined(DEBUG_FILE)
  if (flog) fclose(flog);
#endif
}

void debug(char *fmt, ...)
{
#if DEBUG
  char s[200];
  va_list  args;

#if defined (DEBUG_PROMPT)
  strcpy(s, "debug: ");
  va_start(args, fmt);
  vsnprintf(s+sizeof("debug: ")-1, sizeof(s)-sizeof("debug: "), fmt, args);
  va_end( args );
#else
  va_start(args, fmt);
  _vsnprintf(s, sizeof(s), fmt, args);
  va_end( args );
#endif

#ifdef UNIX
  fprintf(stdout, s);
#else
  yfprintf(stdout, s);
#endif

#if defined(DEBUG_FILE)
  if (flog)
  {
    fprintf(flog, s);
    fflush(flog);
  }
#endif
#endif
}

void debug_ex(char *prefix, char *fmt, ...)
{
#if DEBUG
  char s[200];
  int n;
  va_list  args;

  sprintf(s, "%s debug: ", prefix);
  va_start(args, fmt);
  n = strlen(s);
  _vsnprintf(s+n, sizeof(s)-n, fmt, args);
  va_end( args );
#ifdef UNIX
  fprintf(stdout, s);
#else
  yfprintf(stdout, s);
#endif
#endif
}

void info(char *fmt, ...)
{
#if DEBUG
  char s[200];
  va_list  args;

  strcpy(s, "info:  ");
  va_start(args, fmt);
  _vsnprintf(s+sizeof("info:  ")-1, sizeof(s)-sizeof("info:  "), fmt, args);
  va_end( args );
#ifdef UNIX
  fprintf(stdout, s);
#else
  yfprintf(stdout, s);
#endif
#endif
}

void error(char *fmt, ...)
{
#if DEBUG
  char s[200];
  va_list  args;

  sprintf(s, "error: ");
  va_start(args, fmt);
  _vsnprintf(s+sizeof("error: ")-1, sizeof(s)-sizeof("error: "), fmt, args);
  va_end( args );
#ifdef UNIX
  fprintf(stderr, s);
#else
  yfprintf(stderr, s);
#endif
#endif
}
