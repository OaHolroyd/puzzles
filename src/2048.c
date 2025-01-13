#include <stdio.h>

#include "core/test.h"


int main(int argc, char const *argv[]) {
  fprintf(stderr, "%s\n", argv[0]);

  test_func(2048);

  return 0;
}
