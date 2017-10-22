#pragma once
#include <memory>
#include <utility>
#include <string>

namespace dice {

class Engine
{
public:
    Engine(const std::string& filename);
    ~Engine();
    std::string login(const std::string& body);
    std::string createGame(const std::string& body);
    std::string joinGame(const std::string& body);
    std::string status(const std::string& body) const;
    std::string getGames() const;
    
    void save();
    void save2();
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace dice
