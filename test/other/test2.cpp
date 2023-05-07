#include <iostream>
#include "func.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_CASE("func")
{
    CHECK(func() == 1);
    CHECK(func() == 2);
    CHECK(func() == 1);
}
