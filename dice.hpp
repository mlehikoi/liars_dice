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
public:
    static Dice& instance();

    virtual int roll() const;// override;
};

} // namespace dice
