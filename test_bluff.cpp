#include "bid.hpp"
#include "helpers.hpp"
#include "engine.hpp"
#include "json.hpp"
#include "game.hpp"

#include <unordered_set>

#include "gtest/gtest.h"

#include <rapidjson/document.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <thread>

using namespace std;

namespace {
using namespace dice;

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
    char tmp[4096];
    std::strcpy(tmp, prefix);
    std::strcat(tmp, "XXXXXX");
    auto f = ::mkstemp(tmp);
    ::close(f);
    return tmp;
}

class TmpFile
{
    const std::string filename_;
public:
    TmpFile(const std::string& filename) : filename_{filename} {}
    ~TmpFile() { std::remove(filename_.c_str()); }
    TmpFile(TmpFile&& other) : filename_{other.filename_} {}
    auto str() { return filename_; }
};

inline auto tmpCopy(const char* src, const char* targetPrefix)
{
    auto tmp = tmpName(targetPrefix);
    dice::dump(tmp, dice::slurp(src));
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
    "id": "000",
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
    cout << "Result: " << result << endl;
    EXPECT_TRUE(doc["success"].GetBool());
    std::string id = doc["id"].GetString();

    auto game = json::Json({
        {"id", id},
        {"game", "game"}
    }).str();
    result = e.createGame(game);
    cout << "Result: " << result << endl;
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc["success"].GetBool());
    
    auto game2 = json::Json({
        {"id", id},
        {"game", "game2"}
    }).str();
    result = e.createGame(game2);
    //cout << "Result: " << result << endl;
    doc.Parse(result.c_str());
    EXPECT_FALSE(doc["success"].GetBool());
    EXPECT_STREQ("ALREADY_JOINED", doc["error"].GetString());

    // Try to create the same name game again
    result = e.login(json::Json({"name", "anon2"}).str());
    doc.Parse(result.c_str());
    EXPECT_TRUE(doc["success"].GetBool());
    //cout << "Result: " << result << endl;
    id = doc["id"].GetString();
    
    auto game3 = json::Json({
        {"id", id},
        {"game", "game"}
    }).str();
    result = e.createGame(game3);
    //cout << "Result: " << result << endl;
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
    auto tmp = tmpCopy("../test-game.json", "./.json");
    dice::Engine e{tmp.str()};
    
    const auto doc = dice::parse(e.getGames());
    //dice::prettyPrint(doc);
    auto games = doc.GetArray();
    ASSERT_EQ(2, games.Size());
    
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

    e.save2();
    EXPECT_TRUE(fileExists(tmp));
    cout << slurp(tmp) << endl;
    const auto doc = dice::parse(dice::slurp(tmp));
    EXPECT_TRUE(doc["players"].IsArray());
    EXPECT_EQ(4, doc["players"].Size());

    EXPECT_EQ(origData, dice::slurp(tmp));
}

TEST(EngineTest, JoinGame) {
    auto tmp = tmpCopy("../test-game.json", "./.json");
    dice::Engine e{tmp.str()};

    EXPECT_EQ(2, dice::parse(e.getGames()).Size());

    auto ret = e.joinGame(R"#({
        "id_": "1",
        "game": "final"
    })#");

    //cout << "ret:" << ret << endl;
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("PARSE_ERROR", dice::parse(ret)["error"].GetString());
    
    cout << "joinGame" << endl;
    ret = e.joinGame(R"#({
        "id": "5",
        "game": "final"
    })#");
    cout << "ret:" << ret << endl;
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("NO_PLAYER", dice::parse(ret)["error"].GetString());
    
    ret = e.joinGame(R"#({
        "id": "4",
        "game": "final"
    })#");
    //cout << "ret:" << ret << endl;
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("ALREADY_JOINED", dice::parse(ret)["error"].GetString());
    EXPECT_STREQ("semifinal", dice::parse(ret)["game"].GetString());

    ret = e.joinGame(R"#({
        "id": "3",
        "game": "quarterfinal"
    })#");
    //cout << "ret:" << ret << endl;
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("NO_GAME", dice::parse(ret)["error"].GetString());
    
    ret = e.joinGame(R"#({
        "id": "3",
        "game": "semifinal"
    })#");
    //cout << "ret:" << ret << endl;
    EXPECT_TRUE(dice::parse(ret)["success"].GetBool());
    
    const auto doc = dice::parse(e.getGames());
    //dice::prettyPrint(doc);
    EXPECT_EQ(2, doc.Size());
    EXPECT_STREQ("semifinal", doc[1]["game"].GetString());
    EXPECT_STREQ("ann", doc[1]["players"][0].GetString());
    EXPECT_STREQ("ken", doc[1]["players"][1].GetString());
}

class MockDice : public IDice
{
public:
    int roll() const { return 1; }
};
TEST(EngineGame, TestConstruct) {
    MockDice d;
    dice::Game g{"final", d};
    g.addPlayer("joe");
    g.addPlayer("ann");
    g.addPlayer("mary");
    
    EXPECT_TRUE(g.startGame());
    EXPECT_FALSE(g.startGame());
    
    EXPECT_TRUE(g.startRound());
    EXPECT_FALSE(g.startRound());
    
    auto s = g.getStatus("joe");
    auto doc = parse(g.getStatus("joe"));
    cout << s << endl;
    EXPECT_EQ(0, doc["turn"].GetInt());
    
    EXPECT_STREQ("joe", doc["players"][0]["name"].GetString());
    EXPECT_EQ(5, doc["players"][0]["hand"].Size());
    
    EXPECT_STREQ("ann", doc["players"][1]["name"].GetString());
    EXPECT_EQ(5, doc["players"][1]["hand"].Size());
    
    EXPECT_STREQ("mary", doc["players"][2]["name"].GetString());
    EXPECT_EQ(5, doc["players"][2]["hand"].Size());
    
    // Can't challenge yet
    ASSERT_FALSE(g.challenge("joe"));

    // Not mary's turn
    EXPECT_FALSE(g.bid("mary", 1, 1));
    
    // Joe can play
    EXPECT_TRUE(g.bid("joe", 1, 1));
    doc = parse(g.getStatus("joe"));
    EXPECT_EQ(1, doc["turn"].GetInt());
    
    // Hold on Mary, it's still not your turn
    EXPECT_FALSE(g.bid("mary", 1, 2));
    
    // Ann can bid, but it should be higher bid than previous
    EXPECT_FALSE(g.bid("ann", 1, 1));
    EXPECT_TRUE(g.bid("ann", 1, 2));
    doc = parse(g.getStatus("joe"));
    EXPECT_EQ(2, doc["turn"].GetInt());
    
    // Okay Mary, now it's your turn
    EXPECT_TRUE(g.bid("mary", 5, 4));
    doc = parse(g.getStatus("joe"));
    EXPECT_EQ(0, doc["turn"].GetInt());
    
    EXPECT_TRUE(g.bid("joe", 5, 5));
    EXPECT_FALSE(g.bid("ann", 3, 5));
    EXPECT_TRUE(g.bid("ann", 3, STAR));
    
    EXPECT_FALSE(g.challenge("joe"));
    
    cout << g.getStatus("mary") << endl;
    EXPECT_TRUE(g.challenge("mary"));
    cout << g.getStatus("mary") << endl;
    doc = parse(g.getStatus("mary"));
    
    ASSERT_EQ(2, doc["turn"].GetInt());
    
    EXPECT_STREQ("joe", doc["players"][0]["name"].GetString());
    EXPECT_EQ(       0, doc["players"][0]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][0]["winner"].GetBool());
    EXPECT_FALSE(       doc["players"][0]["loser"].GetBool());
    
    EXPECT_STREQ("ann", doc["players"][1]["name"].GetString());
    EXPECT_EQ(      -3, doc["players"][1]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][1]["winner"].GetBool());
    EXPECT_TRUE(        doc["players"][1]["loser"].GetBool());
    
    EXPECT_STREQ("mary", doc["players"][2]["name"].GetString());
    EXPECT_EQ(        0, doc["players"][2]["adjustment"].GetInt());
    EXPECT_TRUE(         doc["players"][2]["winner"].GetBool());
    EXPECT_FALSE(        doc["players"][2]["loser"].GetBool());

    EXPECT_FALSE(g.bid("mary", 12, 1));
    EXPECT_TRUE(g.startRound());
    EXPECT_TRUE(g.bid("mary", 12, 1));
    EXPECT_TRUE(g.challenge("joe"));
    
    cout << g.getStatus("mary") << endl;
    doc = parse(g.getStatus("mary"));

    EXPECT_EQ(2, doc["turn"].GetInt());
    
    EXPECT_STREQ("joe", doc["players"][0]["name"].GetString());
    EXPECT_EQ(      -1, doc["players"][0]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][0]["winner"].GetBool());
    EXPECT_TRUE(        doc["players"][0]["loser"].GetBool());
    
    EXPECT_STREQ("ann", doc["players"][1]["name"].GetString());
    EXPECT_EQ(      -1, doc["players"][1]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][1]["winner"].GetBool());
    EXPECT_FALSE(        doc["players"][1]["loser"].GetBool());
    
    EXPECT_STREQ("mary", doc["players"][2]["name"].GetString());
    EXPECT_EQ(        0, doc["players"][2]["adjustment"].GetInt());
    EXPECT_TRUE(         doc["players"][2]["winner"].GetBool());
    EXPECT_FALSE(        doc["players"][2]["loser"].GetBool());
    
    EXPECT_TRUE(g.startRound());
    EXPECT_TRUE(g.bid("mary", 10, 1));
    EXPECT_TRUE(g.bid("joe", 10, 2));
    
    EXPECT_TRUE(g.challenge("ann"));
    
    cout << g.getStatus("mary") << endl;
    doc = parse(g.getStatus("mary"));

    EXPECT_EQ(1, doc["turn"].GetInt());
    
    EXPECT_STREQ("joe", doc["players"][0]["name"].GetString());
    EXPECT_EQ(     -10, doc["players"][0]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][0]["winner"].GetBool());
    EXPECT_TRUE(        doc["players"][0]["loser"].GetBool());
    
    EXPECT_STREQ("ann", doc["players"][1]["name"].GetString());
    EXPECT_EQ(       0, doc["players"][1]["adjustment"].GetInt());
    EXPECT_TRUE(        doc["players"][1]["winner"].GetBool());
    EXPECT_FALSE(       doc["players"][1]["loser"].GetBool());
    
    EXPECT_STREQ("mary", doc["players"][2]["name"].GetString());
    EXPECT_EQ(        0, doc["players"][2]["adjustment"].GetInt());
    EXPECT_FALSE(        doc["players"][2]["winner"].GetBool());
    EXPECT_FALSE(        doc["players"][2]["loser"].GetBool());
    
    EXPECT_TRUE(g.startRound());
    cout << g.getStatus("mary") << endl;
    
    EXPECT_TRUE(g.bid("ann", 1, 1));
    EXPECT_TRUE(g.bid("mary", 1, 2));
    
    cout << g.getStatus("mary") << endl;

    EXPECT_TRUE(g.bid("ann", 1, 3));
    cout << g.getStatus("mary") << endl;
    
    EXPECT_TRUE(g.challenge("mary"));
    cout << g.getStatus("mary") << endl;
    doc = parse(g.getStatus("mary"));
    ASSERT_EQ(2, doc["turn"].GetInt());
    
    EXPECT_STREQ("joe", doc["players"][0]["name"].GetString());
    EXPECT_EQ(       0, doc["players"][0]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][0]["winner"].GetBool());
    EXPECT_FALSE(       doc["players"][0]["loser"].GetBool());

    EXPECT_STREQ("ann", doc["players"][1]["name"].GetString());
    EXPECT_EQ(      -1, doc["players"][1]["adjustment"].GetInt());
    EXPECT_FALSE(       doc["players"][1]["winner"].GetBool());
    EXPECT_TRUE(        doc["players"][1]["loser"].GetBool());

    EXPECT_STREQ("mary", doc["players"][2]["name"].GetString());
    EXPECT_EQ(        0, doc["players"][2]["adjustment"].GetInt());
    EXPECT_TRUE(         doc["players"][2]["winner"].GetBool());
    EXPECT_FALSE(        doc["players"][2]["loser"].GetBool());
    
    // EXPECT_TRUE(g.startRound());
    // cout << g.getStatus("mary") << endl;
}

TEST(EngineGame, Save) {
    auto tmp = tmpCopy("../test-game.json", "./.json");
    auto txt0 = slurp("../final.json");
    auto doc = parse(txt0);
    auto game = Game::fromJson(doc);
    ASSERT_TRUE(game);
    ASSERT_EQ("final", game->name());

    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
    game->serialize(w, "");
    cout << s.GetString() << endl;
    ASSERT_STREQ(txt0.c_str(), s.GetString());
}

TEST(BidTest, TestBids) {
    Bid b{1, 1};
    ASSERT_TRUE(Bid(1, 1) < Bid(1, 2));
    ASSERT_FALSE(Bid(1, 2) < Bid(1, 2));
    ASSERT_FALSE(Bid(1, 2) < Bid(1, 1));

    ASSERT_TRUE(Bid(1, 1) < Bid(1, STAR));
    ASSERT_TRUE(Bid(1, 5) < Bid(1, STAR));
    
    ASSERT_TRUE(Bid(1, STAR) < Bid(2, 1));
    ASSERT_FALSE(Bid(2, 1) < Bid(1, STAR));
    
    ASSERT_TRUE(Bid(3, 5) < Bid(4, 1));
    ASSERT_FALSE(Bid(4, 1) < Bid(3, 5));
    
    ASSERT_TRUE(Bid(11, 5) < Bid(6, STAR));
    ASSERT_TRUE(Bid(11, 5) < Bid(7, STAR));
    ASSERT_FALSE(Bid(11, 5) < Bid(5, STAR));
    ASSERT_TRUE(Bid(6, STAR) < Bid(12, 1));
}

TEST(BidTest, TestChallenge) {
    std::vector<int> hand = { 1, 1, 2, 2, 2, STAR, STAR };
    EXPECT_EQ(3, Bid(1, 1).challenge(hand));
    EXPECT_EQ(0, Bid(4, 1).challenge(hand));
    EXPECT_EQ(-1, Bid(5, 1).challenge(hand));
    
    EXPECT_EQ(4, Bid(1, 2).challenge(hand));
    EXPECT_EQ(0, Bid(5, 2).challenge(hand));
    EXPECT_EQ(-5, Bid(10, 2).challenge(hand));
    
    EXPECT_EQ(1, Bid(1, STAR).challenge(hand));
    EXPECT_EQ(0, Bid(2, STAR).challenge(hand));
    EXPECT_EQ(-8, Bid(10, STAR).challenge(hand));
}
} // Unnamed namespace