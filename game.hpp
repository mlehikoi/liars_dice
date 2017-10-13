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
auto doRoll()
{
    static std::random_device r;
    static std::default_random_engine e1(r());
    std::uniform_int_distribution<int> uniform_dist(1, 6);
    return uniform_dist(e1);
}

class Player
{
    std::string name_;
    std::vector<int> dice_;
public:
    Player(const std::string& name)
      : name_{name},
        dice_(5)
    {
        roll();
        std::cout << "dice: " << dice_.size() << std::endl;
        for (const auto& d : dice_) std::cout << d << std::endl;
    }
    
    void roll()
    {
        for (auto& d : dice_)
        {
            d = doRoll();
        }
    }
    const auto& name() { return name_; }
    bool isPlaying() const { return !dice_.empty(); }
};


class Game
{
    std::string game_;
    std::vector<Player> players_;
    int round_;
    int turn_;
    Bid currentBid_;
    bool roundStarted_;

public:
    Game(const std::string& game)
      : game_{game},
        players_{},
        round_{},
        turn_{0},
        currentBid_{},
        roundStarted_{false}
    {
    }
    
    void addPlayer(const std::string& player)
    {
        players_.push_back({player});
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
        std::cout << "turn: " << turn_ << std::endl;
        std::cout << player << " vs " << currentPlayer().name()  << std::endl;
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
        // Bid bid{n, face};
        // if (currentBid_ < bid)
        // {
        //     currentBid_ = bid;
        //     return true;
        // }
        return false;
    }
};

} // namespace game