#include "helpers.hpp"

#include <unordered_set>

#include "gtest/gtest.h"

#include <iostream>

namespace {

TEST(UuidTest, All) {
    std::unordered_set<std::string> s;
    
    for (int i = 0; i < 1'000; ++i)
    {
        auto id = dice::uuid();
        EXPECT_EQ(36, id.size());
        EXPECT_TRUE(s.insert(id).second);
    }
    EXPECT_EQ(1'000, s.size());
}

TEST(SlurpTest, All) {
    auto data = dice::slurp("../hex.dat");
    EXPECT_EQ(4, data.size());
    EXPECT_EQ('1', data[0]);
    EXPECT_EQ('2', data[1]);
    EXPECT_EQ('\0', data[2]);
    EXPECT_EQ('4', data[3]);
    
    data = dice::slurp("not-existing.txt");
    EXPECT_EQ(0, data.size());
}

TEST(ExtensionTest, All) {
    EXPECT_EQ("jpg", dice::getExtension("foo.jpg"));
    EXPECT_EQ("jpeg", dice::getExtension("foo.jpeg"));
    EXPECT_EQ("png", dice::getExtension("foo.jpg.png"));
}

TEST(ContentTypeTest, All) {
    EXPECT_EQ("image/jpeg", dice::getContentType("foo.jpg"));
    EXPECT_EQ("text/html; charset=utf-8", dice::getContentType("index.html"));
    EXPECT_EQ("image/png", dice::getContentType("foo.jpg.png"));
}

} // Unnamed namespace