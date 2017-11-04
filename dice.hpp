#pragma once

namespace dice {

class IDice
{
public:
    virtual ~IDice();
    virtual int roll() const = 0;
};

class Dice : public IDice
{
    static const IDice* dice_;
public:
    static void setInstance(const IDice* dice);
    static const IDice& instance();

    virtual int roll() const override;
};

} // namespace dice
