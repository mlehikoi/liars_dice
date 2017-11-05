#include "game.hpp"

#include "dice.hpp"

#include <algorithm>

namespace dice {

int Game::getOffset() const
{
    std::vector<int> commonHand;
    for (const auto& p : players_)
    {
        commonHand.insert(commonHand.end(), p.hand().begin(), p.hand().end());
    }
    return currentBid_.challenge(commonHand);
}

const char* Game::toString(State state)
{
    switch (state)
    {
    case GAME_NOT_STARTED: return "GAME_NOT_STARTED";
    case GAME_STARTED: return "GAME_STARTED";
    case ROUND_STARTED: return "ROUND_STARTED";
    case CHALLENGE: return "CHALLENGE";
    case GAME_FINISHED: return "GAME_FINISHED";
    }
}

Game::State Game::fromString(const std::string& str)
{
    if (str == "GAME_NOT_STARTED") return GAME_NOT_STARTED;
    if (str == "GAME_STARTED") return GAME_STARTED;
    if (str == "ROUND_STARTED") return ROUND_STARTED;
    if (str == "CHALLENGE") return CHALLENGE;
    if (str == "GAME_FINISHED") return GAME_FINISHED;
    return GAME_NOT_STARTED;
}

std::tuple<int, bool, bool> Game::getResult(int offset, const Player& player) const
{
    //std::cout << player.name() << " ";
    //std::cout << &player << " " << bidder_ << " " << challenger() << std::endl;
    if (&player == bidder_)
        return std::make_tuple(offset < 0 ? offset : 0, offset >= 0, offset < 0);
    if (&player == challenger())
        return std::make_tuple(offset >= 0 ? std::min(-1, -offset) : 0, offset < 0, offset >= 0);
    return std::make_tuple(offset == 0 ? -1 : 0, false, false);
}

void Game::setTurn(const Player& player)
{
    //std::cout << "Set turn " << player.name() << std::endl;
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

Game::Game(const std::string& game)
    : game_{game},
    players_{},
    turn_{0},
    currentBid_{},
    diceRoll_{Dice::instance()},
    state_{GAME_NOT_STARTED}
{
}

Game::Game(const std::string& game, const std::string& player)
  : game_{game},
    players_{},
    turn_{0},
    currentBid_{},
    diceRoll_{Dice::instance()},
    state_{GAME_NOT_STARTED}
{
    players_.emplace_back(player);
}

RetVal Game::addPlayer(const std::string& player)
{
    switch (state_)
    {
    case GAME_NOT_STARTED:
    case GAME_FINISHED:
        if (players_.size() >= 8) return Error{"TOO_MANY_PLAYERS"};
        players_.emplace_back(player);
        return Success{};
    case GAME_STARTED:
    case ROUND_STARTED:
    case CHALLENGE:
        return Error{"GAME_IN_PROGRESS"};
    }
}

void Game::removePlayer(const Player& player)
{
    auto it = std::find(players_.begin(), players_.end(), player);
    if (it != players_.end())
    {
        players_.erase(it);
    }
}

RetVal Game::startGame()
{
    if (state_ != GAME_NOT_STARTED && state_ != GAME_FINISHED)
        return Error{"GAME_ALREADY_STARTED"};
    state_ = GAME_STARTED;
    return Success{};
}

RetVal Game::startRound()
{
    switch (state_)
    {
    case CHALLENGE:
    {
        const auto offset = getOffset();
        for (auto& p : players_)
        {
            auto result = getResult(offset, p);
            p.remove(static_cast<std::size_t>(-std::get<0>(result)));
        }
    }
    [[clang::fallthrough]];
    case GAME_STARTED:
        state_ = ROUND_STARTED;
        currentBid_ = Bid{};
        for (auto& p : players_) p.roll();
        return Success{};
    case GAME_NOT_STARTED:
    case ROUND_STARTED:
    case GAME_FINISHED:
        return Error{"GAME_NOT_STARTED"};
    }
}

void Game::nextPlayer()
{
    ++turn_;
    auto nPlayers = players_.size();
    for (size_t i = 0; i < players_.size(); ++turn_)
    {
        turn_ %= nPlayers;
        if (players_[static_cast<std::size_t>(turn_)].isPlaying()) return;
    }
    assert(false);
}

RetVal Game::bid(const std::string& player, int n, int face)
{
    if (state_ != ROUND_STARTED) return Error{"ROUND_NOT_STARTED"};
    if (player != currentPlayer().name()) return Error{"NOT_YOUR_TURN"};
    Bid bid{n, face};
    if (!bid.valid()) return Error{"INVALID_BID"};
    if (!(currentBid_ < bid)) return Error{"TOO_LOW_BID"};

    currentBid_ = bid;
    bidder_ = &currentPlayer();
    currentPlayer().bid(bid);
    nextPlayer();
    return Success{};
}

RetVal Game::challenge(const std::string& player)
{
    if (currentBid_ == Bid{}) return Error{"NOTHING_TO_CHALLENGE"};
    if (player != currentPlayer().name()) return Error{"NOT_YOUR_TURN"};
    
    const auto offset = getOffset();
    
    // Is it done...
    int numPlayers = 0;
    for (const auto& p : players_)
    {
        const auto result = getResult(offset, p);
        //std::cout << p.name() << " " << p.hand().size() << " " << std::get<0>(result) << std::endl;
        
        const auto numDice = static_cast<int>(p.hand().size());
        const auto numDiceRemoved = -std::get<0>(result);
        const int diceLeft = numDice - numDiceRemoved;
        if (diceLeft > 0)
        {
            ++numPlayers;
        }
    }
    assert(numPlayers >= 1);
    state_ = numPlayers == 1 ? GAME_FINISHED : CHALLENGE;
    
    challenger_ = &currentPlayer();
    if (offset >= 0)
    {
        setTurn(*bidder_);
    }
    return Success{};
}

std::string Game::getStatus(const std::string& player)
{
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
    serialize(w, player);
    return s.GetString();
}

void Game::serialize(json::Writer& writer, const std::string& name) const
{
    using namespace json;
    Object(writer, [=](auto& w)
    {
        KeyValue(w, "game", game_);
        KeyValue(w, "state", toString(state_));
        KeyValue(w, "turn", turn_);
        KeyValueF(w, "bid", [=](auto& w)
        {
            currentBid_.serialize(w);
        });

        ArrayW(w, "players", [=](auto& w1)
        {
            const auto offset = getOffset();
            const bool allRevealed = state_ == CHALLENGE || state_ == GAME_FINISHED;
            for (const auto& p : players_)
            {
                if (allRevealed)
                    p.serialize(w1, getResult(offset, p));
                else
                    p.serialize(w1, name);
            }
        });
    });
}

void Game::serializeGameInfo(json::Writer& w) const
{
    json::Object(w, [this](auto& w)
    {
        json::KeyValue(w, "game", game_);
        json::ArrayW(w, "players", [this](auto& w)
        {
            for (const auto& player : players_)
            {
                w.String(player.name().c_str());
            }
        });
    });
}

std::unique_ptr<Game> Game::fromJson(const rapidjson::Value& v)
{
    using namespace json;
    auto game = std::make_unique<Game>(getString(v, "game"));
    game->turn_ = getInt(v, "turn");
    game->state_ = fromString(getString(v, "state"));
    game->currentBid_ = Bid::fromJson(getValue(v, "bid"));
    for (const auto& jplayer : getArray(v, "players"))
    {
        game->players_.push_back(Player::fromJson(jplayer));
    }
    return game;
}

} // namespace dice
