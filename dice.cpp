#include "dice.hpp"

#include <random>

namespace dice {

IDice::~IDice() {}

void Dice::setInstance(const IDice* dice) { dice_ = dice; }
const IDice& Dice::instance()
{
    if (dice_) return *dice_;
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
const IDice* Dice::dice_{nullptr};

} // namespace dice
