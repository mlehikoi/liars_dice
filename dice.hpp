#pragma once

namespace dice {

/// Abstraction for dice roll
class IDice
{
public:
    /// Destructor
    virtual ~IDice();
    /// Roll the dice
    /// @return face of the dice [1, 6]
    virtual int roll() const = 0;
};

/// Dice roller randomizing the dice roll
class Dice : public IDice
{
    static const IDice* dice_;
public:
    /// User this if you want to replace the standard functionality
    /// @param dice [in] dice abraction to use for rolling the dice
    static void setInstance(const IDice* dice);
    /// Get the configured dice instance
    /// @return IDice object
    static const IDice& instance();

    /// @see IDice::roll
    virtual int roll() const override;
};

} // namespace dice
