#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <libintl.h>
#include <stdarg.h>

# define _(String) gettext (String)
# define N_(String) gettext_noop (String)


#include "../common/elf.h"

void
error (const char *message, ...)
{
  va_list args;

  /* Try to keep error messages in sync with the program's normal output.  */
  fflush (stdout);

  va_start (args, message);
  //fprintf (stderr, _("%s: Error: "), program_name);
  vfprintf (stderr, message, args);
  va_end (args);
}

int main (int argc, char ** argv){
    struct stat statBuf;
    const char * inputFileName = argv[1];

    if (stat (inputFileName, &statBuf) < 0){
        if (errno == ENOENT)
            //fprintf (stderr, "No such file: %s\n", inputFileName);
            error (_("Could not locate '%s'.  System error message: %s\n"), inputFileName, strerror (errno));
        else
            error (_("Could not locate '%s'.  System error message: %s\n"), inputFileName, strerror (errno));
        return FALSE;
    }

    Elf64_Ehdr eHdr;
    return 0;
}
