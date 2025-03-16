#include <gtest/gtest.h>

#include <memory/shared_ptr.hpp>

TEST(shared_ptr, basic) {
  auto p = std::make_shared<int>();
  EXPECT_EQ(*p, int{});

  auto p2 = p;
  *p = 2;

  EXPECT_EQ(*p, 2);
}
