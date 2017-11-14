#include "game.hpp"
#include "test/test_helpers.hpp"

#include "gtest/gtest.h"

namespace dice {
namespace {

TEST(GameTest, Save) {
    auto tmp = tmpCopy("../test-game.json", "./.json");
    auto txt0 = slurp("../final.json");
    auto doc = parse(txt0);
    auto game = Game::fromJson(doc);
    ASSERT_TRUE(game);
    ASSERT_EQ("final", game->name());

    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
    game->serialize(w, "");
    ASSERT_STREQ(txt0.c_str(), s.GetString());
}

TEST(GameTest, JoinWhenGameInProgress) {
    Game game{"the-game", "joe"};
    ASSERT_TRUE(game.addPlayer("ann"));
    ASSERT_TRUE(game.startGame());
    ASSERT_TRUE(game.startRound());
    auto rv = game.addPlayer("mary");
    ASSERT_FALSE(rv);
    ASSERT_STREQ("GAME_IN_PROGRESS", json::getString(parse(rv), "error").c_str());
}

TEST(GameTest, StartGameWithOnePlayer) {
    Game game{"joe"};
    const auto rv = game.startGame();
    ASSERT_FALSE(rv);
    ASSERT_STREQ("NOT_ENOUGH_PLAYERS", json::getString(parse(rv), "error").c_str());
}

} // Unnamed namespace
} // namespace dice
