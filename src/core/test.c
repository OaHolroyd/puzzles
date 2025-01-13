#include "test.h"

#include <stdio.h>

#include <notcurses/notcurses.h>


/* ========================================================================== */
/*   FUNCTION DEFINITIONS                                                     */
/* ========================================================================== */
void test_func(int i) {
  fprintf(stderr, "test_func: %d\n", i);
}
