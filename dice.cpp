#include "dice.hpp"

#include <random>

namespace dice {

IDice::~IDice() {}

Dice& Dice::instance()
{
    static Dice dice;
    return dice;
}

int Dice::roll() const
{
    static std::random_device r;
    static std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(1, 6);
    return uniform_dist(e1);
}

} // namespace dice
