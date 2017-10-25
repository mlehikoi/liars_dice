#include "engine.hpp"

#include "game.hpp"
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

class Error
{
    json::Json doc_;
public:
    Error(const char* msg) : doc_({
        {"success", false},
        {"error", msg}
    })
    {
    }

    auto str() const { return doc_.str(); }
    operator std::string() const { return str(); }
};

class Success
{
public:
    auto str() const { return "{\"success\": true}"; }
    operator std::string() const { return str(); }
};

class Engine::Impl
{
    const std::string filename_;
    // id - player
    std::map<std::string, std::string> players_;
    // Id -> Game name
    std::unordered_map<std::string, std::string> joinedGames_;
    std::map<std::string, std::shared_ptr<Game>> games_;
    
public:
    Impl(const std::string& filename)
      : filename_{filename},
        players_{}
    {
        load();
    }
    
    // -> login
    std::string login(const std::string& body)
    {
        auto doc = parse(body);
        const auto name = getString(doc, "name");
        if (name.empty()) return Error{"INVALID_FORMAT"};
        for (const auto idName : players_)
        {
            if (idName.second == name)
            {
                return Error{"PLAYER_EXISTS"};
            }
        }
        const auto id = uuid();
        const auto ret = players_.insert({id, name});
        assert(ret.second);
        return json::Json({
            {"success", true},
            {"id", id}
        }).str();
    }
    
    std::string createGame(const std::string& body)
    {
        // @TODO Move parse to json
        auto doc = parse(body);
        const auto id = getString(doc, "id");
        const auto game = getString(doc, "game");
        if (game.empty() || id.empty())
        {
            return Error{"PARSE_ERROR"};
        }

        std::cout << "createGame " << id << " " << game << std::endl;
        // Make sure player exists
        auto playerIt = players_.find(id);
        if (playerIt == players_.end()) return Error{"NO_PLAYER"};

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
            return Error{"GAME_EXISTS"};
        }
        joinedGames_.insert({id, game});
        //games_.insert({game, {playerIt->second}});
        //@TODO game with player name
        auto ret = games_.insert({game, std::make_unique<Game>(game)});
        assert(ret.second);
        ret.first->second->addPlayer(playerIt->second);
        return json::Json({"success", true}).str();
    }
    
    std::string joinGame(const std::string& body)
    {
        const auto doc = parse(body);
        const auto id = getString(doc, "id");
        const auto game = getString(doc, "game");
        if (game.empty() || id.empty())
        {
            return json::Json({
                {"success", false},
                {"error", "PARSE_ERROR"}
            }).str();
        }
        std::cout << "joinGame " << id << " " << game << std::endl;
        // Make sure player exists
        auto playerIt = players_.find(id);
        if (playerIt == players_.end()) return Error{"NO_PLAYER"};
        
        { // And isn't already joined in a game
            auto it = joinedGames_.find(id);
            if (it != joinedGames_.end())
                return json::Json({
                    {"success", false},
                    {"error", "ALREADY_JOINED"},
                    {"game", it->second}
                }).str();
        }
        
        auto gameIt = games_.find(game);
        if (gameIt == games_.end())
        {
            return json::Json({
                {"success", false},
                {"error", "NO_GAME"}
            }).str();
        }
        //@TODO TOO_MANY_PLAYES
        joinedGames_.insert({id, game});
        gameIt->second->addPlayer(playerIt->second);
        return json::Json({"success", true}).str();
    }

    std::string startGame(const std::string& body)
    {
        const auto id = getString(parse(body), "id");        
        if (id.empty()) return Error{"PARSE_ERROR"};
        
        const auto pit = players_.find(id);
        if (pit == players_.end()) return Error{"NO_PLAYER"};
        
        const auto jit = joinedGames_.find(id);
        if (jit == joinedGames_.end()) return Error{"NOT_JOINED"};
        
        auto git = games_.find(jit->second);
        if (git == games_.end()) return Error{"FATAL"};

        return git->second->startGame() ? Success{}.str() : Error{"COULD_NOT_START_GAME"}.str();
    }

    std::string startRound(const std::string& body)
    {
        std::cout << "startRound " << body << std::endl;
        const auto id = getString(parse(body), "id");        
        if (id.empty()) return Error{"PARSE_ERROR"};
        
        const auto pit = players_.find(id);
        if (pit == players_.end()) return Error{"NO_PLAYER"};
        
        const auto jit = joinedGames_.find(id);
        if (jit == joinedGames_.end()) return Error{"NOT_JOINED"};
        
        auto git = games_.find(jit->second);
        if (git == games_.end()) return Error{"FATAL"};

        return git->second->startRound() ? Success{}.str() : Error{"COULD_NOT_START_ROUND"}.str();
    }

    std::string status(const std::string& body) const
    {
        std::cout << "status " << body << std::endl;

        auto doc = dice::parse(body);
        if (doc.IsObject() &&
            doc.HasMember("id") && doc["id"].IsString())
        {
            const std::string id = doc["id"].GetString();
            const auto pit = players_.find(id);
            if (pit == players_.end())
            {
                return Error{"ID_NOT_FOUND"};
            }
            const auto name = pit->second;

            rapidjson::StringBuffer s;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};

            w.StartObject();
            w.Key("success"); w.Bool(true);
            w.Key("id"); w.String(id.c_str());
            w.Key("name"); w.String(name.c_str());
            const auto jit = joinedGames_.find(id);
            std::cout << "Trying to find games..." << std::endl;
            if (jit != joinedGames_.end())
            {
                std::cout << "There was a game joined..." << std::endl;
                const auto git = games_.find(jit->second);
                if (git != games_.end())
                {
                    std::cout << "And the game existed.." << std::endl;
                    //@TODO Don't reveal other dice
                    w.Key("game");
                    git->second->serialize(w);
                }
            }
            w.EndObject();
            return s.GetString();
            
        }
        return Error{"INVALID_FORMAT"};
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
            for (const auto& player : kv.second->players())
            {
                w.String(player.name().c_str());
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
        //std::cout << s.GetString() << std::endl;
        dump(filename_, s.GetString());
    }

    void save2()
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        
        w.StartObject();
        w.Key("players");
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
        w.Key("games");
        w.StartArray();
        for (const auto& it : games_)
        {
            it.second->serialize(w);
        }

        w.EndArray();
        w.EndObject();
        std::cout << "***Games***" << std::endl;
        std::cout << s.GetString() << std::endl;
        dump(filename_, s.GetString());
    }

private:
    void load()
    {
        auto doc = parse(slurp(filename_));
        prettyPrint(doc);
        // Players...
        if (doc.IsObject() && doc.HasMember("players") && doc["players"].IsArray())
        {
            for (const auto& el : doc["players"].GetArray())
            {
                const auto id = getString(el, "id");
                const auto name = getString(el, "name");
                if (id.empty() || name.empty()) continue;
                
                players_.insert({id, name});
                const auto game = getString(el, "game");
                if (!game.empty())
                {
                    joinedGames_.insert({id, game});
                }
            }
        }
        // Games...
        if (doc.IsObject() && doc.HasMember("games") && doc["games"].IsArray())
        {
            for (const auto& jgame : doc["games"].GetArray())
            {
                auto game = Game::fromJson(jgame);
                if (game)
                {
                    games_.insert({game->name(), std::move(game)});
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
    return impl_->login(body);
}

std::string Engine::createGame(const std::string& body)
{
    return impl_->createGame(body);
}

std::string Engine::joinGame(const std::string& body)
{
    return impl_->joinGame(body);
}

std::string Engine::startGame(const std::string& body)
{
    return impl_->startGame(body);
}

std::string Engine::startRound(const std::string& body)
{
    return impl_->startRound(body);
}

std::string Engine::status(const std::string& body) const
{
    return impl_->status(body);
}

std::string Engine::getGames() const
{
    return impl_->getGames();
}

void Engine::save()
{
    impl_->save();
}

void Engine::save2()
{
    impl_->save2();
}

} // namespace dice

