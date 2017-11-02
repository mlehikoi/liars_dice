#pragma once
#include "bid.hpp"
#include "dice.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace dice {

class Player
{
    std::string name_;
    std::vector<int> hand_;
    Bid bid_;
    const IDice& diceRoll_;
    // Player()
    //   : name_{},
    //     diceRoll_{Dice::instance()}
    // {

    // }
public:
    Player(const std::string& name, const IDice& diceRoll)
      : name_{name},
        hand_(5),
        bid_{},
        diceRoll_{diceRoll}
    {
    }

    Player(Player&& other)
      : name_{std::move(other.name_)},
        hand_{std::move(other.hand_)},
        bid_{std::move(other.bid_)},
        diceRoll_{other.diceRoll_}
    {
    }

    Player& operator=(Player&& other)
    {
        name_ = {std::move(other.name_)};
        hand_ = {std::move(other.hand_)};
        bid_ = {std::move(other.bid_)};
        return *this;
    }
    bool operator==(const Player& other) { return name_ == other.name_; }
    operator bool() const { return !name_.empty(); }
    
    const auto& hand() const { return hand_; }
    const auto& bid() const { return bid_; }
    const auto& name() const { return name_; }
    bool isPlaying() const { return !hand_.empty(); }

    void roll();
    void bid(const Bid& bid);
    void remove(std::size_t adjustment);

    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, const std::string& player) const;
    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
                   const std::tuple<int, bool, bool>& result) const;

    static Player fromJson2(const rapidjson::Value& v);

private:
    void doSerialize(
        rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
        const std::string* player,
        const std::tuple<int, bool, bool>* result) const;
};

} // namespace dice
