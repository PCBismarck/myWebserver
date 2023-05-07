#include <iostream>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "test2.cpp"

// int func()
// {
//     return 1;
// }

TEST_CASE("func")
{
    CHECK(func() == 1);
    CHECK(func() == 2);
    CHECK(func() == 1);
}
