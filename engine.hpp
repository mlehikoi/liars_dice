#pragma once
#include <memory>
#include <utility>
#include <string>

namespace dice {

class Engine
{
public:
    /// Construct Engine
    /// @param filename [in] json file for loading and saving
    Engine(const std::string& filename) noexcept;
    /// Destructor
    ~Engine() noexcept;

    /// Create a new user account
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string login(const std::string& body) noexcept;

    /// Create a new game
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string createGame(const std::string& body) noexcept;

    /// Join an existing game
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string joinGame(const std::string& body) noexcept;

    /// Start the game you have joined
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string startGame(const std::string& body) noexcept;

    /// Start a round of the game you have joined
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string startRound(const std::string& body) noexcept;

    /// Bid for a round in game you've joined
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string bid(const std::string& body) noexcept;

    /// Challenge a bid in round of game you've joined
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string challenge(const std::string& body) noexcept;

    /// Get status for a player
    /// @param body [in] json containing required input
    /// @return json indicating success of failure
    std::string status(const std::string& body) const noexcept;

    /// Get current games and players
    /// @return json containing list of games and players or error
    std::string getGames() const noexcept;

    /// Save state to the file given in constructor
    void save() noexcept;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace dice
