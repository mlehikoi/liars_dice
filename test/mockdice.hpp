#pragma once
#include "dice.hpp"

namespace {
class MockDice : public dice::IDice
{
public:
    int roll() const override { return 1; }
};
} // Unnamed namespace
