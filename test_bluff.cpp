#include "bid.hpp"
#include "dice.hpp"
#include "helpers.hpp"
#include "engine.hpp"
#include "json.hpp"
#include "game.hpp"
#include "test/mockdice.hpp"
#include "test/test_helpers.hpp"

#include <unordered_set>

#include "gtest/gtest.h"

#include <rapidjson/document.h>

#include <cstdio>
#include <functional>
#include <iostream>
#include <thread>

using namespace std;

using namespace dice;

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

TEST(EngineTest, LoginInvalid) {
    dice::Engine e{""};
    auto anon = R"#({"nam": "anon"})#";
    auto result = e.login(anon);
    auto doc = parse(result);
    EXPECT_FALSE(doc["success"].GetBool());
    EXPECT_STREQ(doc["error"].GetString(), "PARSE_ERROR");
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
    //cout << "Result: " << result << endl;
    EXPECT_TRUE(doc["success"].GetBool());
    std::string id = doc["id"].GetString();

    auto game = json::Json({
        {"id", id},
        {"game", "game"}
    }).str();
    result = e.createGame(game);
    //cout << "Result: " << result << endl;
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

TEST(EngineTest, LoadExtraPlayerInGame) {
    auto tmp = tmpCopy("../game-with-extra-player.json", "./.json");
    dice::Engine e{tmp.str()};
    
    const auto doc = dice::parse(e.getGames());
    auto games = doc.GetArray();
    ASSERT_EQ(1, games.Size());
    
    auto status = e.status("joe");
    EXPECT_EQ("final", games[0]["game"]);
    EXPECT_EQ(1, games[0]["players"].Size());
    EXPECT_STREQ("joe", games[0]["players"][0].GetString());
}

TEST(EngineTest, PlayerInTwoGames) {
    auto tmp = tmpCopy("../player-in-two-games.json", "./.json");
    dice::Engine e{tmp.str()};
    
    const auto doc = dice::parse(e.getGames());
    auto games = doc.GetArray();
    ASSERT_EQ(2, games.Size());
    
    auto status = e.status("joe");
    EXPECT_EQ("final", games[0]["game"]);
    EXPECT_EQ(1, games[0]["players"].Size());
    EXPECT_STREQ("joe", games[0]["players"][0].GetString());

    EXPECT_EQ("semifinal", games[1]["game"]);
    EXPECT_EQ(1, games[1]["players"].Size());
    EXPECT_STREQ("mary", games[1]["players"][0].GetString());
}

TEST(EngineTest, GameWithNoPlayers) {
    auto tmp = tmpCopy("../game-with-no-players.json", "./.json");
    dice::Engine e{tmp.str()};
    
    const auto doc = dice::parse(e.getGames());
    auto games = doc.GetArray();
    ASSERT_EQ(0, games.Size());
}

TEST(EngineTest, GameInvalidJson) {
    auto tmp = tmpCopy("../game-invalid.json", "./.json");
    dice::Engine e{tmp.str()};
    
    const auto doc = dice::parse(e.getGames());
    auto games = doc.GetArray();
    ASSERT_EQ(1, games.Size());
    EXPECT_EQ("semifinal", games[0]["game"]);
    EXPECT_EQ(1, games[0]["players"].Size());
    EXPECT_STREQ("mary", games[0]["players"][0].GetString());
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
    //cout << slurp(tmp) << endl;
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
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("PARSE_ERROR", dice::parse(ret)["error"].GetString());
    
    ret = e.joinGame(R"#({
        "id": "5",
        "game": "final"
    })#");
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("NO_PLAYER", dice::parse(ret)["error"].GetString());
        
    ret = e.joinGame(R"#({
        "id": "4",
        "game": "final"
    })#");
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("ALREADY_JOINED", dice::parse(ret)["error"].GetString());

    ret = e.joinGame(R"#({
        "id": "3",
        "game": "quarterfinal"
    })#");
    EXPECT_FALSE(dice::parse(ret)["success"].GetBool());
    EXPECT_STREQ("NO_GAME", dice::parse(ret)["error"].GetString());
    
    ret = e.joinGame(R"#({
        "id": "3",
        "game": "semifinal"
    })#");
    EXPECT_TRUE(dice::parse(ret)["success"].GetBool());
    
    const auto doc = dice::parse(e.getGames());
    EXPECT_EQ(2, doc.Size());
    EXPECT_STREQ("semifinal", doc[1]["game"].GetString());
    EXPECT_STREQ("ann", doc[1]["players"][0].GetString());
    EXPECT_STREQ("ken", doc[1]["players"][1].GetString());
}

TEST(EngineTest, TestBid) {
    auto tmp = tmpCopy("../test-game.json", "./.json");
    dice::Engine e{tmp.str()};

    EXPECT_EQ(2, dice::parse(e.getGames()).Size());
    
    auto ret = e.startGame(R"#( {"id": "1"} )#");
    ASSERT_TRUE(dice::parse(ret)["success"].GetBool());

    // PARSE_ERROR
    ret = e.bid(R"#({
        "id": "1",
        "n": 5
    })#");
    ASSERT_FALSE(parse(ret)["success"].GetBool());
    ASSERT_STREQ("PARSE_ERROR", parse(ret)["error"].GetString());

    // NO_PLAYER
    ret = e.bid(R"#({
        "id": "10",
        "n": 5,
        "face": 5
    })#");
    ASSERT_FALSE(parse(ret)["success"].GetBool());
    ASSERT_STREQ("NO_PLAYER", parse(ret)["error"].GetString());

    // NO_JOINED
    ret = e.bid(R"#({
        "id": "3",
        "n": 5,
        "face": 5
    })#");
    ASSERT_FALSE(parse(ret)["success"].GetBool());
    ASSERT_STREQ("NOT_JOINED", parse(ret)["error"].GetString());

    // SUCCESS
    ret = e.bid(R"#({
        "id": "1",
        "n": 4,
        "face": 5
    })#");
    ASSERT_TRUE(dice::parse(ret)["success"].GetBool());

    ret = e.status(R"#( {"id": "1"} )#");
    //cout << "Ret: " << ret << endl;
    auto doc = parse(ret);
    ASSERT_TRUE(doc["success"].GetBool());
    ASSERT_STREQ("1", doc["id"].GetString());
    ASSERT_STREQ("joe", doc["name"].GetString());
    ASSERT_STREQ("final", doc["game"]["game"].GetString());
    ASSERT_STREQ("ROUND_STARTED", doc["game"]["state"].GetString());
    ASSERT_EQ(1, doc["game"]["turn"].GetInt());
    ASSERT_EQ(4, doc["game"]["bid"]["n"].GetInt());
    ASSERT_EQ(5, doc["game"]["bid"]["face"].GetInt());

    // joe
    ASSERT_EQ(2, doc["game"]["players"].Size());
    ASSERT_STREQ("joe", doc["game"]["players"][0]["name"].GetString());
    ASSERT_EQ(4, doc["game"]["players"][0]["bid"]["n"].GetInt());
    ASSERT_EQ(5, doc["game"]["players"][0]["bid"]["face"].GetInt());
    ASSERT_EQ(5, doc["game"]["players"][0]["hand"].Size());
    for (int i = 0; i < 5; ++i)
    {
        const auto dice = doc["game"]["players"][0]["hand"][i].GetInt();
        ASSERT_TRUE(0 < dice && dice <= 6);
    }

    // mary
    ASSERT_STREQ("mary", doc["game"]["players"][1]["name"].GetString());
    ASSERT_EQ(0, doc["game"]["players"][1]["bid"]["n"].GetInt());
    ASSERT_EQ(0, doc["game"]["players"][1]["bid"]["face"].GetInt());
    ASSERT_EQ(5, doc["game"]["players"][1]["hand"].Size());
    for (int i = 0; i < 5; ++i)
    {
        ASSERT_EQ(0, doc["game"]["players"][1]["hand"][i].GetInt());
    }
}

TEST(EngineTest, TestChallenge) {
    auto tmp = tmpCopy("../test-game.json", "./.json");
    dice::Engine e{tmp.str()};
    EXPECT_EQ(2, dice::parse(e.getGames()).Size());
    
    // start game and round
    auto ret = e.startGame(R"#( {"id": "1"} )#");
    ASSERT_TRUE(dice::parse(ret)["success"].GetBool());

    // Make a bid
    ret = e.bid(R"#({
        "id": "1",
        "n": 5,
        "face": 5
    })#");
    ASSERT_TRUE(dice::parse(ret)["success"].GetBool());

    // PARSE_ERROR
    ret = e.challenge(R"#( {"id_": "1"} )#");
    ASSERT_FALSE(parse(ret)["success"].GetBool());
    ASSERT_STREQ("PARSE_ERROR", parse(ret)["error"].GetString());

    // NO_PLAYER
    ret = e.challenge(R"#( {"id": "10"} )#");
    ASSERT_FALSE(parse(ret)["success"].GetBool());
    ASSERT_STREQ("NO_PLAYER", parse(ret)["error"].GetString());

    // NO_JOINED
    ret = e.bid(R"#( {"id": "3"} )#");
    ASSERT_FALSE(parse(ret)["success"].GetBool());
    ASSERT_STREQ("NOT_JOINED", parse(ret)["error"].GetString());

    // SUCCESS
    ret = e.challenge(R"#( {"id": "2"} )#");
    ASSERT_TRUE(dice::parse(ret)["success"].GetBool());
}

TEST(EngineGame, TestConstruct) {
    MockDice d;
    Dice::setInstance(&d);
    AtEnd ae{[]{ Dice::setInstance(nullptr); }};
    dice::Game g{"final"};
    g.addPlayer("joe");
    g.addPlayer("ann");
    g.addPlayer("mary");
    
    EXPECT_TRUE(g.startGame());
    EXPECT_FALSE(g.startGame());
    
    EXPECT_TRUE(g.startRound());
    EXPECT_FALSE(g.startRound());
    
    auto s = g.getStatus("joe");
    auto doc = parse(g.getStatus("joe"));
    //cout << s << endl;
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
    
    //cout << g.getStatus("mary") << endl;
    EXPECT_TRUE(g.challenge("mary"));
    //cout << g.getStatus("mary") << endl;
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
    
    //cout << g.getStatus("mary") << endl;
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
    
    //cout << g.getStatus("mary") << endl;
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
    //cout << g.getStatus("mary") << endl;
    
    EXPECT_TRUE(g.bid("ann", 1, 1));
    EXPECT_TRUE(g.bid("mary", 1, 2));
    
    //cout << g.getStatus("mary") << endl;

    EXPECT_TRUE(g.bid("ann", 1, 3));
    //cout << g.getStatus("mary") << endl;
    
    EXPECT_TRUE(g.challenge("mary"));
    //cout << g.getStatus("mary") << endl;
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
    //cout << s.GetString() << endl;
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