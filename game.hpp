#pragma once

#include "player.hpp"

#include "bid.hpp"
#include "json.hpp"
#include "helpers.hpp" // RetVal

#include <memory>
#include <random>
#include <string>
#include <vector>

namespace dice {

class IDice;

template<typename T>
inline T set(T& value, T newValue)
{
    T prev = value;
    value = newValue;
    return prev;
}

/**
 * Represents a game with players and the state of the game.
 * 
 * @startuml
 * [*] --> GAME_NOT_STARTED
 * GAME_NOT_STARTED --> GAME_STARTED : startGame
 * GAME_STARTED --> ROUND_STARTED : startRound
 * ROUND_STARTED --> ROUND_STARTED : bid
 * ROUND_STARTED --> CHALLENGE : challenge [remaining player > 1]
 * ROUND_STARTED --> GAME_FINISHED : challenge [remaining player == 1]
 * CHALLENGE --> ROUND_STARTED : startRound
 * GAME_FINISHED --> GAME_STARTED : startGame
 * @enduml
 */
class Game
{
    std::string game_;
    std::vector<Player> players_;
    //int round_;
    int turn_;
    Bid currentBid_;
    //bool roundStarted_;
    const IDice& diceRoll_;
    enum State
    {
        GAME_NOT_STARTED,
        GAME_STARTED,
        ROUND_STARTED,
        CHALLENGE,
        GAME_FINISHED
    } state_;

    // Used for challenge
    const Player* bidder_;
    const Player* challenger_;

    int getOffset() const;

    static const char* toString(State state);
    static State fromString(const std::string& str);
    
    auto& currentPlayer() { return players_[static_cast<std::size_t>(turn_)]; }
    const auto& currentPlayer() const { return players_[static_cast<std::size_t>(turn_)]; }
    const auto& challenger() const { return challenger_; }
    
    std::tuple<int, bool, bool> getResult(int offset, const Player& player) const;
    
    void setTurn(const Player& player);
    void nextPlayer();
public:
    /// Construct a new game
    /// @param game [in] name of the game
    explicit Game(const std::string& game);
    /// Construct a new game
    /// @param game [in] name of the game
    /// @param player [in] first player in the game
    Game(const std::string& game, const std::string& player);
    /// No copying
    Game(Game&&) = delete;

    /// @return name of the game
    const auto& name() { return game_; }

    /// Add player to the game
    /// @param player [in] player's name
    /// @return json indicating the success of the operation
    RetVal addPlayer(const std::string& player);

    /// Remove player from the game
    /// @param player [in] player to remove
    void removePlayer(const Player& player);

    /// @return players in the game
    const auto& players() const { return players_; }
    
    /// Start the game
    /// @return json indicating the success of the operation
    RetVal startGame();

    /// Start the round of the game
    /// @return json indicating the success of the operation
    RetVal startRound();

    /// Make a bid
    /// @param player [in] who's bidding
    /// @param n [in] how many
    /// @param face [in] WTF = what's the face
    /// @return json indicating the success of the operation
    RetVal bid(const std::string& player, int n, int face);

    /// Challenge previous player's bid
    /// @param player [in] the challenger
    /// @return json indicating the success of the operation
    RetVal challenge(const std::string& player);

    /// @return status of the game for the given player
    std::string getStatus(const std::string& player);

    /**
     * Serialize engine state to given writer. If round is still in progress,
     * only given player's dice are "shown".
     *
     * @param w [out] state is serialized here
     * @param name [in] who's dice to show if round is in progress. If empty,
     *     show all dice.
     */
    void serialize(json::Writer& w, const std::string& name) const;

    /**
     * Serialize information about all games: name, players.
     *
     * @param w [out] state is serialized here
     */
    void serializeGameInfo(json::Writer& w) const;

    /// Load game from json
    /// @param v [in] json where to serialize from
    /// @throws ParseError if invalid format
    static std::unique_ptr<Game> fromJson(const rapidjson::Value& v);
};

} // namespace dice
