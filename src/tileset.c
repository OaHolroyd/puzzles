#include <stdio.h>

#include "core/test.h"


int main(int argc, char const *argv[]) {
  fprintf(stderr, "filename: %s\n", argv[0]);

  test_func(argc);

  return 0;
}
