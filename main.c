#include "detest.h"

/* Example tests */

DETEST_TEST(math, addition)
{
  ASSERT_EQ(2 + 2, 4);
}

DETEST_TEST(math, subtraction)
{
  ASSERT_EQ(10 - 3, 7);
}

DETEST_TEST(math, failing)
{
  ASSERT_EQ(10 - 3, 8);
}

DETEST_TEST(strings, equality)
{
  ASSERT_EQ_STR("hello", "hello");
}

DETEST_TEST(strings, inequality)
{
  ASSERT_NE(strcmp("foo", "bar"), 0);
}

DETEST_DEFINITIONS;
#include "detest.h"

int main(int argc, char **argv)
{
  return detest_run(argc, argv);
}
