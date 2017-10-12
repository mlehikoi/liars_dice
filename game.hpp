#pragma once
#include "bid.hpp"

#include <random>
#include <string>
#include <vector>

namespace dice {

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
    
    bool isPlaying() const { return !dice_.empty(); }
};


class Game
{
    std::string game_;
    std::vector<Player> players_;
    int round_;
    int turn_;
    Bid currentBid_;
public:
    Game(const std::string& game)
      : game_{game},
        players_{},
        round_{},
        turn_{},
        currentBid_{}
    {
    }
    
    void addPlayer(const std::string& player)
    {
        players_.push_back({player});
    }
    
    void startGame()
    {
        ++round_;
        for (auto& p : players_) p.roll();
    }
    
    bool bid(int n, int face)
    {
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