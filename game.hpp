#pragma once
#include "player.hpp"

#include "bid.hpp"
#include "json.hpp"
#include "helpers.hpp"

#include <rapidjson/prettywriter.h>

#include <random>
#include <string>
#include <vector>

namespace dice {

template<typename T>
inline T set(T& value, T newValue)
{
    T prev = value;
    value = newValue;
    return prev;
}

class Game
{
    std::string game_;
    std::vector<Player> players_;
    int round_;
    int turn_;
    Bid currentBid_;
    bool roundStarted_;
    const IDice& diceRoll_;
    // enum State
    // {
    //     GAME_NOT_STARTED,
    //     GAME_STARTED,
    //     ROUND_STARTED,
    //     CHALLENGE,
    //     GAME_FINISHED
    // } state_;
    MAKE_ENUM(GameState,
        GAME_NOT_STARTED,
        GAME_STARTED,
        ROUND_STARTED,
        CHALLENGE,
        GAME_FINISHED
    );

    GameState state_;

    // Used for challenge
    const Player* bidder_;
    const Player* challenger_;
    
    auto getOffset() const
    {
        //if (state_ == CHALLENGE)
        {
            std::vector<int> commonHand;
            for (const auto p : players_)
            {
                commonHand.insert(commonHand.end(), p.hand().begin(), p.hand().end());
            }
            return currentBid_.challenge(commonHand);
        }
        return 0;
    }

    static auto fromString(const std::string str)
    {
        if (str == "GAME_NOT_STARTED") return GAME_NOT_STARTED;
        if (str == "GAME_STARTED") return GAME_STARTED;
        if (str == "ROUND_STARTED") return ROUND_STARTED;
        if (str == "CHALLENGE") return CHALLENGE;
        if (str == "GAME_FINISHED") return GAME_FINISHED;
        return GAME_NOT_STARTED;
    }
    
    auto& currentPlayer() { return players_[turn_]; }
    const auto& currentPlayer() const { return players_[turn_]; }
    
    const auto challenger() const { return challenger_; }
    
    auto getResult(int offset, const Player& player) const
    {
        std::cout << player.name() << " ";
        std::cout << &player << " " << bidder_ << " " << challenger() << std::endl;
        if (&player == bidder_)
            return std::make_tuple(offset < 0 ? offset : 0, offset >= 0, offset < 0);
        if (&player == challenger())
            return std::make_tuple(offset >= 0 ? std::min(-1, -offset) : 0, offset < 0, offset >= 0);
        return std::make_tuple(offset == 0 ? -1 : 0, false, false);
    }
    
    void setTurn(const Player& player)
    {
        std::cout << "Set turn " << player.name() << std::endl;
        int i = 0;
        for (const auto& p : players_)
        {
            if (&p == &player)
            {
                turn_ = i;
                return;
            }
            ++i;
        }
    }

public:
    Game(const std::string& game, const IDice& diceRoll = Dice::instance())
      : game_{game},
        players_{},
        round_{},
        turn_{0},
        currentBid_{},
        roundStarted_{false},
        diceRoll_{diceRoll},
        state_{GAME_NOT_STARTED}
    {
    }

    const auto& name() { return game_; }

    void addPlayer(const std::string& player)
    {
        players_.push_back({player, diceRoll_});
    }

    auto players() const { return players_; }
    
    bool startGame()
    {
        if (state_ != GAME_NOT_STARTED && state_ != GAME_FINISHED)
            return false;
        state_ = GAME_STARTED;
        return true;
    }
    
    bool startRound()
    {
        switch (state_)
        {
        case CHALLENGE:
        {
            const auto offset = getOffset();
            for (auto& p : players_)
            {
                auto result = getResult(offset, p);
                p.remove(-std::get<0>(result));
            }
        }
        //[[clang::fallthrough]];
        case GAME_STARTED:
            state_ = ROUND_STARTED;
            currentBid_ = Bid{};
            for (auto& p : players_) p.roll();
            return true;
        default:
            return false;
        }
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
        std::cout << toString(state_) << " " << currentPlayer().name() << std::endl;
        if (state_ != ROUND_STARTED) return false;
        if (player == currentPlayer().name())
        {
            Bid bid{n, face};
            if (currentBid_ < bid)
            {
                currentBid_ = bid;
                //@TODO set bit to player
                bidder_ = &currentPlayer();
                currentPlayer().bid(bid);
                nextPlayer();
                return true;
            }
            std::cout << "Too low bid" << std::endl;
            return false;
        }
        return false;
    }
    
    bool challenge(const std::string player)
    {
        if (currentBid_ == Bid{})
            return false;
        if (player != currentPlayer().name())
            return false;
        const auto offset = getOffset();
        
        // Is it done...
        int numPlayers = 0;
        for (const auto& p : players_)
        {
            auto result = getResult(offset, p);
            //std::cout << p.name() << " " << p.hand().size() << " "
            if (p.hand().size() > -std::get<0>(result)) ++numPlayers;
        }
        assert(numPlayers >= 1);
        state_ = numPlayers == 1 ? GAME_FINISHED : CHALLENGE;
        
        challenger_ = &currentPlayer();
        if (offset >= 0)
        {
            setTurn(*bidder_);
        }
        return true;
    }
    
    std::string getStatus(const std::string& player)
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        w.StartObject();
        w.Key("turn"); w.String(currentPlayer().name().c_str());
        w.Key("state"); w.String(toString(state_));

        w.Key("players");
        w.StartArray();
        
        const auto offset = getOffset();
        for (const auto& p : players_)
        {
            if (state_ == CHALLENGE || state_ == GAME_FINISHED)
            {
                auto result = getResult(offset, p);
                std::cout << p.name() << " " << offset << " " << std::get<0>(result) << std::endl;
                p.serialize(w, result);
            }
            else
            {
                p.serialize(w, player);
            }
        }
        w.EndArray();
        w.EndObject();
        return s.GetString();
    }

    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w) const
    {
        w.StartObject();
        w.Key("game");
        w.String(game_.c_str());

        w.Key("state");
        w.String(toString(state_));

        w.Key("turn");
        w.Int(turn_);

        w.Key("bid");
        w.StartObject();
        w.Key("n");
        w.Int(currentBid_.n());
        w.Key("face");
        w.Int(currentBid_.face());
        w.EndObject();

        w.Key("players");
        w.StartArray();
        for (const auto& p : players_)
            p.serialize(w);
        w.EndArray();
        w.EndObject();
    }

    static auto fromJson(const rapidjson::Value& v)
    {
        std::cout << "ParseGame" << std::endl;
        if (v.IsObject() &&
            v.HasMember("game") && v["game"].IsString() &&
            v.HasMember("turn") && v["turn"].IsInt() &&
            v.HasMember("state") && v["state"].IsString() &&
            v.HasMember("bid") && v["bid"].IsObject() &&
            v.HasMember("players") && v["players"].IsArray())
        {
            auto game = std::make_unique<Game>(v["game"].GetString());
            game->turn_ = v["turn"].GetInt();
            game->state_ = fromString(v["state"].GetString());
            for (const auto& jplayer : v["players"].GetArray())
            {
                auto player = Player::fromJson(jplayer);
                if (player)
                {
                    std::cout << "Valid" << std::endl;
                    game->players_.push_back(player);
                }
                else
                {
                    std::cout << "Invalid" << std::endl;
                }
            }
            return game;
        }
        
        return std::unique_ptr<Game>{};
    }
};

} // namespace game