#include "game.hpp"

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
    std::cout << player.name() << " ";
    std::cout << &player << " " << bidder_ << " " << challenger() << std::endl;
    if (&player == bidder_)
        return std::make_tuple(offset < 0 ? offset : 0, offset >= 0, offset < 0);
    if (&player == challenger())
        return std::make_tuple(offset >= 0 ? std::min(-1, -offset) : 0, offset < 0, offset >= 0);
    return std::make_tuple(offset == 0 ? -1 : 0, false, false);
}

void Game::setTurn(const Player& player)
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

Game::Game(const std::string& game, const IDice& diceRoll)
    : game_{game},
    players_{},
    //round_{},
    turn_{0},
    currentBid_{},
    //roundStarted_{false},
    diceRoll_{diceRoll},
    state_{GAME_NOT_STARTED}
{
}

void Game::addPlayer(const std::string& player)
{
    players_.push_back({player, diceRoll_});
}

bool Game::startGame()
{
    if (state_ != GAME_NOT_STARTED && state_ != GAME_FINISHED)
        return false;
    state_ = GAME_STARTED;
    return true;
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
        return Error{"CANNOT_BE_STARTED"};
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
    //@TODO set bit to player
    bidder_ = &currentPlayer();
    currentPlayer().bid(bid);
    nextPlayer();
    return Success{};
}

RetVal Game::challenge(const std::string player)
{
    if (currentBid_ == Bid{}) return Error{"NOTHING_TO_CHALLENGE"};
    if (player != currentPlayer().name()) return Error{"NOT_YOUR_TURN"};
    
    const auto offset = getOffset();
    
    // Is it done...
    int numPlayers = 0;
    for (const auto& p : players_)
    {
        auto result = getResult(offset, p);
        //std::cout << p.name() << " " << p.hand().size() << " "
        if (p.hand().size() > std::size_t(-std::get<0>(result))) ++numPlayers;
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

/**
 * Serialize engine state to give writer. If round is still in progress,
 * only given player's dice are "shown".
 *
 * @param w [out] state is serialized here
 * @param name [in] who's dice to show if round is in progress. If empty,
 *     show all dice.
 */
void Game::serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w, const std::string& name) const
{
    w.StartObject();
    w.Key("game");
    w.String(game_.c_str());

    w.Key("state");
    w.String(toString(state_));

    w.Key("turn");
    std::cout << "turn: " << turn_ << std::endl;
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
    if (state_ == CHALLENGE || state_ == GAME_FINISHED)
    {
        const auto offset = getOffset();
        for (const auto& p : players_)
        {
            auto result = getResult(offset, p);
            p.serialize(w, result);
        }
    }
    else
        for (const auto& p : players_)
            p.serialize(w, name);
    w.EndArray();
    w.EndObject();
}

std::shared_ptr<Game> Game::fromJson(const rapidjson::Value& v)
{
    std::cout << "ParseGame" << std::endl;
    if (v.IsObject() &&
        v.HasMember("game") && v["game"].IsString() &&
        v.HasMember("turn") && v["turn"].IsInt() &&
        v.HasMember("state") && v["state"].IsString() &&
        v.HasMember("bid") && v["bid"].IsObject() &&
        v.HasMember("players") && v["players"].IsArray())
    {
        auto game = std::make_shared<Game>(v["game"].GetString());
        game->turn_ = v["turn"].GetInt();
        game->state_ = fromString(v["state"].GetString());
        game->currentBid_ = Bid::fromJson(v["bid"]);
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
    
    return std::shared_ptr<Game>{};
}

} // namespace dice
