#include "helpers.hpp"
#include "engine.hpp"
#include "json.hpp"

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

TEST(EngineTest, Login) {
    dice::Engine e{""};
    auto anon = R"#(
        {"name": "anon"}
    )#";
    auto result = e.login(anon);
    rapidjson::Document doc;
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc["success"].GetBool());

    result = e.login(anon);
    doc.SetObject();
    doc.Parse(result.c_str());
    EXPECT_FALSE(doc["success"].GetBool());
}

TEST(EngineTest, CreateGameInvalidId) {
    dice::Engine e{""};
    auto game = R"#(
{
    "playerId": "000",
    "game": "game"
}
    )#";
    auto result = e.createGame(game);

    rapidjson::Document doc;
    doc.Parse(result.c_str());
    EXPECT_FALSE(doc["success"].GetBool());
    EXPECT_STREQ("NO_PLAYER", doc["error"].GetString());
}

TEST(EngineTest, CreateGame) {
    dice::Engine e{""};
    auto anon = R"#(
        {"name": "anon"}
    )#";
    auto result = e.login(anon);
    rapidjson::Document doc;
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc["success"].GetBool());
    std::string id = doc["playerId"].GetString();

    auto game = json::Json({
        {"playerId", id},
        {"game", "game"}
    }).str();
    result = e.createGame(game);
    cout << "Result: " << result << endl;
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc["success"].GetBool());
    
    auto game2 = json::Json({
        {"playerId", id},
        {"game", "game2"}
    }).str();
    result = e.createGame(game2);
    cout << "Result: " << result << endl;
    doc.Parse(result.c_str());
    EXPECT_FALSE(doc["success"].GetBool());
    EXPECT_STREQ("ALREADY_JOINED", doc["error"].GetString());

    // Try to create the same name game again
    result = e.login(json::Json({"name", "anon2"}).str());
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc["success"].GetBool());
    cout << "Result: " << result << endl;
    id = doc["playerId"].GetString();
    
    auto game3 = json::Json({
        {"playerId", id},
        {"game", "game"}
    }).str();
    result = e.createGame(game3);
    cout << "Result: " << result << endl;
    doc.Parse(result.c_str());
    EXPECT_FALSE(doc["success"].GetBool());
    EXPECT_STREQ("GAME_EXISTS", doc["error"].GetString());
}

} // Unnamed namespace