#ifndef TESTING_H
#define TESTING_H

#include <stdio.h>


const char *TEST_NAME;
const char *SUBTEST_NAME;
int ERR_COUNT;

#define START_TEST(title) { TEST_NAME = title; SUBTEST_NAME = ""; ERR_COUNT = 0; }


#define SUBTEST(subtitle) { SUBTEST_NAME = subtitle; }


#define REQUIRE(cond) { int l = __LINE__;                                             \
    if (!(cond)) {                                                                    \
      ERR_COUNT++;                                                                    \
      fprintf(stderr, "  %s:%s (line %d) FAILED\n", TEST_NAME, SUBTEST_NAME, l);      \
    } else {                                                                          \
      /* fprintf(stderr, "%s (line %d) PASSED\n", TEST_NAME, l); */                   \
    }                                                                                 \
  }

// as above but exits early if the condition is not met
#define REQUIRE_BARRIER(cond) { int l = __LINE__;                                     \
    if (!(cond)) {                                                                    \
      ERR_COUNT++;                                                                    \
      fprintf(stderr, "  %s:%s (line %d) FAILED\n", TEST_NAME, SUBTEST_NAME, l);      \
      fprintf(stderr, "    ABORTED EARLY DUE TO FAILURE\n");                          \
      return 1;                                                                       \
    } else {                                                                          \
      /* fprintf(stderr, "%s (line %d) PASSED\n", TEST_NAME, l); */                   \
    }                                                                                 \
}


#define END_TEST() { return ERR_COUNT; }


#endif //TESTING_H
