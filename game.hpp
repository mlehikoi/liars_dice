#pragma once
#include "bid.hpp"

#include <random>
#include <string>
#include <vector>

namespace dice {

template<typename T>
T set(T& value, T newValue)
{
    T prev = value;
    value = newValue;
    return prev;
}

class IDice
{
public:
    virtual int roll() const = 0;
};
class Dice : public IDice
{
public:
    int roll() const override
    {
        static std::random_device r;
        static std::default_random_engine e1(r());
        std::uniform_int_distribution<int> uniform_dist(1, 6);
        return uniform_dist(e1);
    }
};

Dice defaultDice_;
class Player
{
    std::string name_;
    std::vector<int> hand_;
    const IDice& diceRoll_;
public:
    Player(const std::string& name, const IDice& diceRoll = defaultDice_)
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
    
    const auto& hand() const { return hand_; }
    
    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, const std::string& player) const
    {
        w.StartObject();
        w.Key("name");
        w.String(name_.c_str());
        w.Key("hand");
        w.StartArray();
        for (auto d : hand_)
        {
            w.Int(name_ == player ? d : 0);
        }
        w.EndArray();
        w.EndObject();
    }
    const auto& name() { return name_; }
    bool isPlaying() const { return !hand_.empty(); }
};


class Game
{
    std::string game_;
    std::vector<Player> players_;
    int round_;
    int turn_;
    Bid currentBid_;
    bool roundStarted_;
    const IDice& diceRoll_;

public:
    Game(const std::string& game, const IDice& diceRoll = defaultDice_)
      : game_{game},
        players_{},
        round_{},
        turn_{0},
        currentBid_{},
        roundStarted_{false},
        diceRoll_{diceRoll}
    {
    }
    

    
    void addPlayer(const std::string& player)
    {
        players_.push_back({player, diceRoll_});
    }
    
    bool startGame()
    {
        if (round_ == 0)
        {
            ++round_;
            return true;
        }
        return false;
    }
    
    bool startRound()
    {
        if (roundStarted_) return false;
        roundStarted_ = true;
        for (auto& p : players_) p.roll();
        return true;
    }
    
    auto& currentPlayer()
    {
        return players_[turn_];
    }
    
    void nextPlayer()
    {
        ++turn_;
        auto nPlayers = players_.size();
        for (size_t i = 0; i < players_.size(); ++turn_)
        {
            turn_ %= nPlayers;
            if (players_[turn_].isPlaying()) return;
        }
        assert(false);
    }
    
    bool bid(const std::string& player, int n, int face)
    {
        //std::cout << "turn: " << turn_ << std::endl;
        //std::cout << player << " vs " << currentPlayer().name()  << std::endl;
        //@TODO return enum to indicate reason of failure
        if (player == currentPlayer().name())
        {
            Bid bid{n, face};
            if (currentBid_ < bid)
            {
                currentBid_ = bid;
                //@TODO set bit to player
                nextPlayer();
                return true;
            }
            return false;
        }
        return false;
    }
    
    bool challenge(const std::string player)
    {
        if (player != currentPlayer().name())
        {
            return false;
        }
        std::vector<int> commonHand;
        for (const auto p : players_)
        {
            commonHand.insert(commonHand.end(), p.hand().begin(), p.hand().end());
        }
        std::cout << "current bid " << currentBid_.n() << " * " << currentBid_.face() << std::endl;
        
        //std::cout << "diff " << 
        return true;
    }
    
    std::string getStatus(const std::string& player)
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        w.StartObject();
        w.Key("turn");
        w.String(currentPlayer().name().c_str());
        w.Key("players");
        w.StartArray();
        
        for (const auto& p : players_)
        {
            p.serialize(w, player);
        }
        w.EndArray();
        w.EndObject();
        return s.GetString();
    }
};

} // namespace game