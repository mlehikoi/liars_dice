#include "helpers.hpp"

#include <unordered_set>

#include "gtest/gtest.h"

#include <iostream>

namespace {

TEST(UuidTest, All) {
    std::unordered_set<std::string> s;
    
    for (int i = 0; i < 1'000; ++i)
    {
        auto id = bluff::uuid();
        EXPECT_EQ(36, id.size());
        EXPECT_TRUE(s.insert(id).second);
    }
    EXPECT_EQ(1'000, s.size());
}

TEST(SlurpTest, All) {
    auto data = bluff::slurp("../hex.dat");
    EXPECT_EQ(4, data.size());
    EXPECT_EQ('1', data[0]);
    EXPECT_EQ('2', data[1]);
    EXPECT_EQ('\0', data[2]);
    EXPECT_EQ('4', data[3]);
}

TEST(ExtensionTest, All) {
    EXPECT_EQ("jpg", bluff::getExtension("foo.jpg"));
    EXPECT_EQ("jpeg", bluff::getExtension("foo.jpeg"));
    EXPECT_EQ("png", bluff::getExtension("foo.jpg.png"));
}

TEST(ContentTypeTest, All) {
    EXPECT_EQ("image/jpeg", bluff::getContentType("foo.jpg"));
    EXPECT_EQ("text/html; charset=utf-8", bluff::getContentType("index.html"));
    EXPECT_EQ("image/png", bluff::getContentType("foo.jpg.png"));
}

} // Unnamed namespace