#include "engine.hpp"

#include "helpers.hpp"
#include "json.hpp"
#include <rapidjson/document.h>

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/error/en.h"

#include <unordered_map>
#include <vector>

using namespace rapidjson;

namespace dice {

class Engine::Impl
{
    const char* filename_;
    // id - player
    std::unordered_map<std::string, std::string> players_;
    std::unordered_map<std::string, std::string> joinedGames_;
    std::unordered_map<std::string, std::vector<std::string>> games_;
    
public:
    Impl(const char* filename)
      : filename_{filename},
        players_{}
    {
        //doc_.Parse(filename_);
    }
    
    // -> login
    std::string login(const std::string name)
    {
        for (const auto idName : players_)
        {
            if (idName.second == name)
            {
                return "";
            }
        }
        const auto id = uuid();
        const auto ret = players_.insert({id, name});
        assert(ret.second);
        return id; 
    }
    
    std::string createGame(const std::string id, const std::string game)
    {
        // Make sure player exists
        if (players_.find(id) == players_.end())
            return json::Json({
                {"success", false},
                {"error", "NO_PLAYER"}
            }).str();
        // And isn't already joined in a game
        auto it = joinedGames_.find(id);
        if (it != joinedGames_.end())
            return json::Json({
                {"success", false},
                {"error", "ALREADY_JOINED"},
                {"game", it->second}
            }).str();
        if (games_.find(game) != games_.end())
        {
            return json::Json({
                {"success", false},
                {"error", "GAME_EXISTS"}
            }).str();
        }
        joinedGames_.insert({id, game});
        games_.insert({game, {id}});
        return json::Json({"success", true}).str();
    }
};

Engine::Engine(const char* filename)
    : impl_{std::make_unique<Impl>(filename)}
{
    
}

Engine::~Engine() = default;

std::string Engine::login(const std::string& body)
{
    rapidjson::Document doc;
    doc.Parse(body.c_str());
    if (doc.IsObject() && doc.HasMember("name") && doc["name"].IsString())
    {
        const auto id = impl_->login(doc["name"].GetString());
        if (id.empty())
        {
            return json::Json({"success", false}).str();
        }
        return json::Json({{"success", true}, {"playerId", id}}).str(); 
    }
    return json::Json({"success", false}).str();
}

std::string Engine::createGame(const std::string& body)
{
    rapidjson::Document doc;
    doc.Parse(body.c_str());
    if (doc.IsObject() &&
        doc.HasMember("game") && doc["game"].IsString() &&
        doc.HasMember("playerId") && doc["playerId"].IsString())
    {
        return impl_->createGame(doc["playerId"].GetString(), doc["game"].GetString());
    }
    return json::Json({
        {"success", false},
        {"error", "PARSE_ERROR"}
    }).str();
}

std::string Engine::joinGame(const std::string& body)
{
    return "";
}

} // namespace dice

