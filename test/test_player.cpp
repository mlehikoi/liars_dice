#include "player.hpp"
#include "json.hpp"

#include "gtest/gtest.h"

namespace dice {
namespace {

TEST(PlayerTest, Equal) {
    Player p1{"joe"};
    Player p2{"joe"};
    Player p3{"ann"};
    ASSERT_TRUE(p1 == p1);
    ASSERT_TRUE(p1 == p2);
    ASSERT_FALSE(p1 == p3);
    ASSERT_FALSE(p2 == p3);
}

TEST(PlayerTest, Hand) {
}

TEST(PlayerTest, Name) {
}

TEST(PlayerTest, Playing) {
}

TEST(PlayerTest, Bid) {
}

TEST(PlayerTest, Roll) {
}

TEST(PlayerTest, Remove) {
}

TEST(PlayerTest, SerializeOne) {
}

TEST(PlayerTest, SerializeAll) {
}

TEST(PlayerTest, SerializeResult) {
}

TEST(PlayerTest, Load) {
    auto p = Player::fromJson(json::Json{
        {
            {"name", "joe"},
            {"hand", json::Array()},
            {"bid", {
                {"face", 4},
                {"n", 5}
            }}
        }
    }.json());
    ASSERT_EQ("joe", p.name());
    ASSERT_TRUE(p.hand().empty());
    ASSERT_EQ(4, p.bid().face());
    ASSERT_EQ(5, p.bid().n());

    bool exceptionThrown = false;
    try {
        p = Player::fromJson(json::Json{
            {
                {"name", "joe"},
                {"hand", json::Array()}
            }
        }.json());
    } catch (const json::ParseError&) {
        exceptionThrown = true;
    }
    ASSERT_TRUE(exceptionThrown);
    ASSERT_EQ("joe", p.name());
    ASSERT_TRUE(p.hand().empty());
    ASSERT_EQ(4, p.bid().face());
    ASSERT_EQ(5, p.bid().n());
}

} // Unnamed namespace
} // namespace dice