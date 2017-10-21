#pragma once
#include <random>

namespace dice {

class IDice
{
public:
    virtual int roll() const = 0;
};
class Dice : public IDice
{
public:
    static auto& instance()
    {
        static Dice dice;
        return dice;
    }

    int roll() const override
    {
        static std::random_device r;
        static std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(1, 6);
        return uniform_dist(e1);
    }
};

}