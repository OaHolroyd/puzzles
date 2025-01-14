#include "testing.h"

#include "core/test.h"

int main(int argc, char const *argv[]) {
  REQUIRE(returns_one() == 1);

  return 0;
}
