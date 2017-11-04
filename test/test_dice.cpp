#include "dice.hpp"
#include "test/mockdice.hpp"

#include "gtest/gtest.h"

#include <unordered_map>

namespace {

TEST(DiceTest, Random) {
    std::unordered_map<int, int> faces;
    for (int i = 0; i < 1000; ++i)
        ++faces[dice::Dice::instance().roll()];
    ASSERT_EQ(6, faces.size());
    for (int i = 1; i <= 6; ++i)
        ASSERT_GT(faces[i], 0);
}

TEST(DiceTest, Fixed) {
    MockDice mockDice;
    dice::Dice::setInstance(&mockDice);
    std::unordered_map<int, int> faces;
    for (int i = 0; i < 1000; ++i)
        ++faces[dice::Dice::instance().roll()];
    ASSERT_EQ(1, faces.size());
    ASSERT_EQ(1000, faces[1]);

    faces.clear();
    ASSERT_TRUE(faces.empty());

    dice::Dice::setInstance(nullptr);
    for (int i = 0; i < 1000; ++i)
        ++faces[dice::Dice::instance().roll()];
    ASSERT_EQ(6, faces.size());
    for (int i = 1; i <= 6; ++i)
        ASSERT_GT(faces[i], 0);
}

} // unnamed namespace
