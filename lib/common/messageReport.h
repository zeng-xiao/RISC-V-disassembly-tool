#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <libintl.h>
#include <stdarg.h>

void error (const char *message, ...)
{
  va_list args;

  /* Try to keep error messages in sync with the program's normal output.  */
  fflush (stdout);

  va_start (args, message);
  fprintf (stderr, _("%s: Error: "), program_name);
  vfprintf (stderr, message, args);
  va_end (args);
}