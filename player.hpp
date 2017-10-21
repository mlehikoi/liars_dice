#pragma once
#include "dice.hpp"

#include <rapidjson/prettywriter.h>

namespace dice {

class Player
{
    std::string name_;
    std::vector<int> hand_;
    const IDice& diceRoll_;
public:
    Player(const std::string& name, const IDice& diceRoll)
      : name_{name},
        hand_(5),
        diceRoll_{diceRoll}
    {
        //roll();
        //std::cout << "dice: " << dice_.size() << std::endl;
        //for (const auto& d : dice_) std::cout << d << std::endl;
    }
    
    void roll()
    {
        for (auto& d : hand_)
        {
            d = diceRoll_.roll();
        }
    }
    
    void remove(std::size_t adjustment)
    {
        const auto size = hand_.size();
        hand_.resize(size - std::min(size, adjustment));
    }
    
    const auto& hand() const { return hand_; }
    
    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w) const
    {
        w.StartObject();
        w.Key("name");
        w.String(name_.c_str());
        w.Key("hand");
        w.StartArray();
        for (auto d : hand_)
        {
            w.Int(d);
        }
        w.EndArray();
        w.EndObject();
    }

    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, const std::string& player) const
    {
        w.StartObject();
        w.Key("name");
        w.String(name_.c_str());
        w.Key("hand");
        w.StartArray();
        for (auto d : hand_)
        {
            w.Int(player.empty() || name_ == player ? d : 0);
        }
        w.EndArray();
        w.EndObject();
    }
    
    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
                   const std::tuple<int, bool, bool>& result) const
    {
        w.StartObject();
        w.Key("name");
        w.String(name_.c_str());

        w.Key("adjustment"); w.Int(std::get<0>(result));
        w.Key("winner"); w.Bool(std::get<1>(result));
        w.Key("loser"); w.Bool(std::get<2>(result));

        w.Key("hand");
        w.StartArray();
        for (auto d : hand_)
        {
            w.Int(d);
        }
        w.EndArray();

        w.EndObject();
    }
    
    const auto& name() const { return name_; }
    bool isPlaying() const { return !hand_.empty(); }
};

}