#ifndef TESTING_H
#define TESTING_H

#include <stdio.h>


char *TEST_NAME;
char *SUBTEST_NAME;
int ERR_COUNT;

#define START_TEST(title) { TEST_NAME = title; SUBTEST_NAME = ""; ERR_COUNT = 0; }
#define SUBTEST(subtitle) { SUBTEST_NAME = subtitle; }
#define REQUIRE(cond) { int l = __LINE__;                                             \
    if (!(cond)) {                                                                    \
      ERR_COUNT++;                                                                    \
      fprintf(stderr, "%s:%s (line %d) FAILED\n", TEST_NAME, SUBTEST_NAME, l);        \
    } else {                                                                          \
      /* fprintf(stderr, "%s (line %d) PASSED\n", TEST_NAME, l); */                   \
    }                                                                                 \
  }
#define END_TEST() { return ERR_COUNT; }


#endif //TESTING_H
