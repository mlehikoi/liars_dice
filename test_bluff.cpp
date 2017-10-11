#include "helpers.hpp"
#include "engine.hpp"
#include "json.hpp"

#include <unordered_set>

#include "gtest/gtest.h"

#include <rapidjson/document.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <thread>

using namespace std;

namespace {

class AtEnd
{
    std::function<void()> f_;
public:
    AtEnd(const std::function<void()>& f) : f_{f} {}
    AtEnd(AtEnd&&) = delete;
    ~AtEnd() { f_(); }
};

inline std::string tmpName(const char* prefix)
{
    char tmp[PATH_MAX];
    std::strcpy(tmp, prefix);
    std::strcat(tmp, "XXXXXX");
    auto f = ::mkstemp(tmp);
    ::close(f);
    return tmp;
}

class TmpFile
{
    const std::string& filename_;
public:
    TmpFile(const std::string& filename) : filename_{filename} {}
    //TmpFile(TmpFile&&) = delete;
    ~TmpFile() { std::remove(filename_.c_str()); }
    auto str() { return filename_; }
};

inline auto tmpCopy(const char* src, const char* targetPrefix)
{
    auto tmp = tmpName(targetPrefix);
    dice::dump(tmp, dice::slurp("../test-game.json"));
    return TmpFile{tmp};
}

inline bool fileExists(const std::string& filename) 
{
    struct stat fileInfo;
    return ::stat(filename.c_str(), &fileInfo) == 0;
}


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
    
    //cout << "Games: " << endl << e.getGames() << endl;
    doc = dice::parse(e.getGames());
    auto games = doc.GetArray();
    EXPECT_EQ(1, games.Size());
    EXPECT_STREQ("anon", games[0]["players"][0].GetString());
    EXPECT_EQ(1, games[0]["players"].Size());
    EXPECT_STREQ("anon", games[0]["players"][0].GetString());
}

TEST(EngineTest, Load) {
    dice::Engine e{"../test-game.json"};
    
    const auto doc = dice::parse(e.getGames());
    dice::prettyPrint(doc);
    auto games = doc.GetArray();
    EXPECT_EQ(2, games.Size());
    EXPECT_EQ("final", games[0]["game"]);
    EXPECT_EQ(2, games[0]["players"].Size());
    EXPECT_STREQ("joe", games[0]["players"][0].GetString());
    EXPECT_STREQ("mary", games[0]["players"][1].GetString());

    EXPECT_EQ("semifinal", games[1]["game"]);
    EXPECT_EQ(1, games[1]["players"].Size());
    EXPECT_STREQ("ann", games[1]["players"][0].GetString());
}

TEST(EngineTest, Save) {
    auto tmp = tmpName("./.json");
    AtEnd ae{[tmp]{ std::remove(tmp.c_str()); }};
    const auto origData = dice::slurp("../test-game.json");
    dice::dump(tmp, origData);

    dice::Engine e{tmp};
    EXPECT_TRUE(fileExists(tmp));
    std::remove(tmp.c_str());
    EXPECT_FALSE(fileExists(tmp));

    e.save();
    EXPECT_TRUE(fileExists(tmp));
    const auto doc = dice::parse(dice::slurp(tmp));
    EXPECT_TRUE(doc.IsArray());
    EXPECT_EQ(3, doc.Size());

    for (const auto& o : doc.GetArray())
    {
        if (o["id"] == "1")
        {
            EXPECT_STREQ("joe", o["name"].GetString());
            EXPECT_STREQ("final", o["game"].GetString());
        }
        else if (o["id"] == "2")
        {
            EXPECT_STREQ("mary", o["name"].GetString());
            EXPECT_STREQ("final", o["game"].GetString());
        }
        else if (o["id"] == "3")
        {
            EXPECT_STREQ("ken", o["name"].GetString());
            EXPECT_FALSE(o.HasMember("game"));
        }
        else
        {
            FAIL();
        }
    }
}

TEST(EngineTest, JoinGame) {
    auto tmp = tmpCopy("../test-game.json", "./.json");
    dice::Engine e{tmp.str()};

    EXPECT_EQ(1, dice::parse(e.getGames()).Size());

    auto ret = e.joinGame(R"#({
        "playerId_": "1",
        "game": "final"
    })#");
    cout << "ret:" << ret << endl;
    EXPECT_STREQ("PARSE_ERROR", dice::parse(ret)["error"].GetString());
    
    ret = e.joinGame(R"#({
        "playerId": "4",
        "game": "final"
    })#");
    cout << "ret:" << ret << endl;
    EXPECT_STREQ("NO_PLAYER", dice::parse(ret)["error"].GetString());
    
    ret = e.joinGame(R"#({
        "playerId": "4",
        "game": "final"
    })#");
    cout << "ret:" << ret << endl;
    EXPECT_STREQ("NO_PLAYER", dice::parse(ret)["error"].GetString());
    
}

} // Unnamed namespace