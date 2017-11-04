#pragma once
#include "bid.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace dice {

class IDice;

class Player
{
    std::string name_;
    std::vector<int> hand_;
    Bid bid_;
public:
    /// Construct a player
    /// @param name [in] name of the player
    Player(const std::string& name)
      : name_{name},
        hand_(5),
        bid_{}
    {
    }
    
    /// Move constructor
    Player(Player&& other)
      : name_{std::move(other.name_)},
        hand_{std::move(other.hand_)},
        bid_{std::move(other.bid_)}
    {
    }

    /// Move assignment
    Player& operator=(Player&& other)
    {
        name_ = {std::move(other.name_)};
        hand_ = {std::move(other.hand_)};
        bid_ = {std::move(other.bid_)};
        return *this;
    }

    /// Equality operator
    bool operator==(const Player& other) { return name_ == other.name_; }
    
    /// @return hand of the player
    const auto& hand() const { return hand_; }

    /// @return bid of the player
    const auto& bid() const { return bid_; }

    /// @return name of the player
    const auto& name() const { return name_; }

    /// @return if the player is still in the game
    bool isPlaying() const { return !hand_.empty(); }

    /// Roll the dice
    void roll();

    /// Make a bid
    void bid(const Bid& bid);

    /// Remove dice from player
    void remove(std::size_t adjustment);

    /// Serailize given player
    /// @param w [out] serialized here
    /// @param player [in] player whose dice are shown
    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, const std::string& player) const;

    /// Serialize players taking into accout result of the round
    /// @param w [out] seriliazed here
    /// @param result [in] results of the round
    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
                   const std::tuple<int, bool, bool>& result) const;

    /// Read player from json
    /// @param v [in] where to read from
    /// @throws ParseError if format is unexpected
    /// @return Player loaded from json
    static Player fromJson(const rapidjson::Value& v);

private:
    void doSerialize(
        rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
        const std::string* player,
        const std::tuple<int, bool, bool>* result) const;
};

} // namespace dice
