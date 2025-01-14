//
// Created by Oscar Holroyd on 14/01/2025.
//

#include "logging.h"

#if !DEBUG

/* if not built in debug mode, just provide stubs that do nothing */
void log_start(const char *file) {}
void log_clear(void) {}
void internal_log(const char *file, const int line, const char *format, ...) {}

#else

#include <stdio.h>
#include <stdarg.h>

const char *LOGGING_FILE = NULL;

void log_start(const char *file) {
  LOGGING_FILE = file;
  log_clear();
}


void log_clear(void) {
  if (LOGGING_FILE) {
    /* open file in write mode to clear the file */
    FILE *fp = fopen(LOGGING_FILE, "w");
    if (fp) {
      fclose(fp);
    }
  }
}


void internal_log(const char *file, const int line, const char *format, ...) {
  /* open file in append mode so it is not cleared */
  FILE *fp = fopen(LOGGING_FILE, "a");
  if (!fp) {
    return;
  }

  va_list args;
  va_start(args, format);

  fprintf(fp, "%s:%d: ", file, line);
  vfprintf(fp, format, args);
  fprintf(fp, "\n");

  va_end(args);

  fclose(fp);
}

#endif
