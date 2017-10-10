#include "engine.hpp"

#include "helpers.hpp"
#include "json.hpp"
#include <rapidjson/document.h>

#include <rapidjson/reader.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/error/en.h>

#include <unordered_map>
#include <vector>

using namespace rapidjson;

namespace dice {

template<typename Doc>
bool hasString(const Doc& doc, const char* member)
{
    return doc.IsObject() &&
           doc.HasMember(member) &&
           doc[member].IsString();
}

template<typename Doc>
std::string getString(const Doc& doc, const char* member)
{
    return hasString(doc, member) ? doc[member].GetString() : "";
}
class Engine::Impl
{
    const std::string filename_;
    // id - player
    std::unordered_map<std::string, std::string> players_;
    std::unordered_map<std::string, std::string> joinedGames_;
    std::unordered_map<std::string, std::vector<std::string>> games_;
    
public:
    Impl(const std::string& filename)
      : filename_{filename},
        players_{}
    {
        load();
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
        auto playerIt = players_.find(id);
        if (playerIt == players_.end())
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
        games_.insert({game, {playerIt->second}});
        return json::Json({"success", true}).str();
    }
    
    std::string getGames() const
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        
        w.StartArray();
        for (const auto& kv : games_)
        {
            w.StartObject();
            w.Key("game");
            w.String(kv.first.c_str());
            
            w.Key("players");
            w.StartArray();
            for (const auto& player : kv.second)
            {
                w.String(player.c_str());
            }
            w.EndArray();
            w.EndObject();
        }
        w.EndArray();
        //std::cout << "games:" << s.GetString() << std::endl;
        return s.GetString();
    }

    void save()
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        
        w.StartArray();
        for (const auto& kv : players_)
        {
            const auto id = kv.first;
            
            w.StartObject();
            w.Key("id");
            w.String(id.c_str());
            
            w.Key("name");
            w.String(kv.second.c_str());
            
            const auto it = joinedGames_.find(id);
            if (it != joinedGames_.end())
            {
                w.Key("game");
                w.String(it->second.c_str());
            }
            w.EndObject();
        }
        w.EndArray();
        std::cout << s.GetString() << std::endl;
        dump(filename_, s.GetString());
    }

private:
    void load()
    {
        auto doc = parse(slurp(filename_));
        prettyPrint(doc);
        if (doc.IsArray())
        {
            for (const auto& el : doc.GetArray())
            {
                const auto id = getString(el, "id");
                const auto name = getString(el, "name");
                if (id.empty() || name.empty()) continue;
                
                players_.insert({id, name});
                const auto game = getString(el, "game");
                if (!game.empty())
                {
                    joinedGames_.insert({id, game});
                    games_[game].push_back(name);
                }
            }
        }
    }
};

Engine::Engine(const std::string& filename)
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
    const auto id = getString(doc, "playerId");
    const auto game = getString(doc, "game");
    if (game.empty() || id.empty())
    {
        return json::Json({
            {"success", false},
            {"error", "PARSE_ERROR"}
        }).str();
    }
    return impl_->createGame(id, game);
}

std::string Engine::joinGame(const std::string& body)
{
    return "";
}

std::string Engine::getGames() const
{
    return impl_->getGames();
}

void Engine::save()
{
    impl_->save();
}

} // namespace dice

