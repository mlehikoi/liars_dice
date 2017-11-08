#include "player.hpp"

#include "bid.hpp"
#include "dice.hpp"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

#include <tuple>

namespace dice {

void Player::roll()
{
    for (auto& d : hand_)
    {
        d = Dice::instance().roll();
    }
    bid_ = {};
}

void Player::bid(const Bid& bid)
{
    bid_ = bid;
}

void Player::remove(std::size_t adjustment)
{
    const auto size = hand_.size();
    hand_.resize(size - std::min(size, adjustment));
}

void Player::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, const std::string& player) const
{
    doSerialize(w, player.empty() ? nullptr : &player, nullptr);
}
    
void Player::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
                const std::tuple<int, bool, bool>& result) const
{
    doSerialize(w, nullptr, &result);
}

void Player::doSerialize(
    rapidjson::PrettyWriter<rapidjson::StringBuffer>& w,
    const std::string* player,
    const std::tuple<int, bool, bool>* result) const
{
    json::Object(w, [=](auto& w)
    {
        json::KeyValue(w, "name", name_);
        json::KeyValueF(w, "bid", [=](auto& w)
        {
            bid_.serialize(w);
        });
    
        if (result)
        {
            json::KeyValue(w, "adjustment", std::get<0>(*result));
            json::KeyValue(w, "winner", std::get<1>(*result));
            json::KeyValue(w, "loser", std::get<2>(*result));
        }
    
        json::ArrayW(w, "hand", [=](auto& w){
            const bool faceShown = (result || !player || *player == name_);
            for (auto d : hand_)
            {
                w.Int(faceShown ? d : 0);
        
            }
        });
    });
}

Player Player::fromJson(const rapidjson::Value& v)
{
    Player player(json::getString(v, "name"));
    player.hand_.clear();
    for (const auto& d : json::getArray(v, "hand"))
    {
        player.hand_.push_back(json::getInt(d));
    }
    player.bid_ = Bid::fromJson(json::getValue(v, "bid"));
    return player;
}

} // namespace dice
