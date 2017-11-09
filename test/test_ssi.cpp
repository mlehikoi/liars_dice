#include "ssi.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std;

TEST(SsiTest, NoInclude) {
    auto str = dice::readHtml("no_include.txt", "../test/");
    ASSERT_STREQ(str.c_str(), "a\nb\nc\n");
}

TEST(SsiTest, OneInclude) {
    auto str = dice::readHtml("one_include.txt", "../test/");
    ASSERT_STREQ(str.c_str(), "0\na\nb\nc\n3\n");
}

TEST(SsiTest, TwoIncludes) {
    auto str = dice::readHtml("two_includes.txt", "../test/");
    ASSERT_STREQ(str.c_str(), "a\nb\nc\n");
}

} // unnamed namespace