#include "httphelpers.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std;

static constexpr auto HEADER = "gzip, deflate, br";

TEST(HttpHelpersTest, ValueFirst) {
    ASSERT_TRUE(dice::hasHttpValue(HEADER, "gzip"));
}

TEST(HttpHelpersTest, ValueInMiddle) {
    ASSERT_TRUE(dice::hasHttpValue(HEADER, "deflate"));
}

TEST(HttpHelpersTest, ValueLast) {
    ASSERT_TRUE(dice::hasHttpValue(HEADER, "br"));
}

TEST(HttpHelpersTest, NoValue) {
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "tar"));
}

TEST(HttpHelpersTest, TooLongKey) {
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "gzip1"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "deflate1"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "br1"));
}

TEST(HttpHelpersTest, TooShortKey) {
    ASSERT_TRUE(dice::hasHttpValue(HEADER, "gzip"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "zip"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "gzi"));
    
    ASSERT_TRUE(dice::hasHttpValue(HEADER, "deflate"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "eflate"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "deflat"));

    ASSERT_TRUE(dice::hasHttpValue(HEADER, "br"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "r"));
    ASSERT_FALSE(dice::hasHttpValue(HEADER, "b"));
}

TEST(HttpHelpersTest, NoSpaces) {
    constexpr auto header = "gzip,deflate,br";
    ASSERT_TRUE(dice::hasHttpValue(header, "gzip"));
    ASSERT_FALSE(dice::hasHttpValue(header, "zip"));
    ASSERT_FALSE(dice::hasHttpValue(header, "gzi"));
    
    ASSERT_TRUE(dice::hasHttpValue(header, "deflate"));
    ASSERT_FALSE(dice::hasHttpValue(header, "eflate"));
    ASSERT_FALSE(dice::hasHttpValue(header, "deflat"));

    ASSERT_TRUE(dice::hasHttpValue(header, "br"));
    ASSERT_FALSE(dice::hasHttpValue(header, "r"));
    ASSERT_FALSE(dice::hasHttpValue(header, "b"));
}

TEST(HttpHelpersTest, ExtraSpaces) {
    constexpr auto header = "gzip , deflate , br";
    ASSERT_TRUE(dice::hasHttpValue(header, "gzip"));
    ASSERT_FALSE(dice::hasHttpValue(header, "zip"));
    ASSERT_FALSE(dice::hasHttpValue(header, "gzi"));
    
    ASSERT_TRUE(dice::hasHttpValue(header, "deflate"));
    ASSERT_FALSE(dice::hasHttpValue(header, "eflate"));
    ASSERT_FALSE(dice::hasHttpValue(header, "deflat"));

    ASSERT_TRUE(dice::hasHttpValue(header, "br"));
    ASSERT_FALSE(dice::hasHttpValue(header, "r"));
    ASSERT_FALSE(dice::hasHttpValue(header, "b"));
}

TEST(HttpHelpersTest, EmptyValue) {
    ASSERT_FALSE(dice::hasHttpValue(HEADER, ""));
}

TEST(HttpHelpersTest, Weight) {
    constexpr auto header = "deflate, gzip;q=1.0, *;q=0.5";
    ASSERT_TRUE(dice::hasHttpValue(header, "deflate"));
    ASSERT_TRUE(dice::hasHttpValue(header, "gzip"));
    //ASSERT_TRUE(dice::hasHttpValue(header, "*"));
    ASSERT_FALSE(dice::hasHttpValue(header, "br"));
}

TEST(HttpHelpersTest, Benchmark) {
    // int matches = 0;
    // constexpr auto header = "deflate, gzip;q=1.0, *;q=0.5";
    // auto t0 = std::chrono::steady_clock::now();
    // for (int i = 0; i < 1000000; ++i)
    // {
    //     matches += dice::hasHttpValue(header, "gzip") ? 1 : 0;
    // }
    // std::chrono::duration<double> dur = std::chrono::steady_clock::now() - t0;
    // cout << "Took " << dur.count() << ", " << matches << endl;
}


} // Unnamed namespace