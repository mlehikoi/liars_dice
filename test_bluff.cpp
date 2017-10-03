#include "helpers.hpp"
#include "engine.hpp"

#include <unordered_set>

#include "gtest/gtest.h"

#include <rapidjson/document.h>

#include <iostream>

using namespace std;

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

TEST(EngineTest, Add) {
    cerr << "Add" << endl;
    dice::Engine e{""};
    auto anon = R"#(
        {"name": "anon"}
    )#";
    auto result = e.login(anon);
    cout << "result: " << result << endl;
    rapidjson::Document doc;
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc.IsObject());
    EXPECT_TRUE(doc["success"].GetBool());
}

} // Unnamed namespace