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

} // Unnamed namespace
} // namespace dice