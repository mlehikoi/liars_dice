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

template<typename Container, typename KeyType>
inline bool hasItem(const Container& c, const KeyType& k)
{
    return c.find(k) != c.end();
}

template<typename Container, typename ValueType>
inline bool hasValue(const Container& c, const ValueType& v)
{
    for (const auto& kv : c)
    {
        if (kv.second == v)
        {
            return true;
        }
    }
    return false;
}


class Engine::Impl
{
    const std::string filename_;
    // id - player
    std::map<std::string, std::string> players_;
    // Id -> Game name
    std::unordered_map<std::string, std::string> joinedGames_;
    std::map<std::string, std::unique_ptr<Game>> games_;

    // Get the game and player for the given doc["id"] or
    // thow an error if anything fails.
    auto getGamePlayer(const rapidjson::Value& doc)
    {
        const std::string id = json::getString(doc, "id");
        
        const auto pit = players_.find(id);
        if (pit == players_.end()) throw json::ParseError{"NO_PLAYER"};
        
        const auto jit = joinedGames_.find(id);
        if (jit == joinedGames_.end()) throw json::ParseError{"NOT_JOINED"};
        
        const auto git = games_.find(jit->second);
        assert(git != games_.end());

        return std::make_pair(git->second.get(), pit->second);
    }

    // Get the player for the given doc["id"]
    const auto& getPlayer(const std::string& id)
    {
        const auto it = players_.find(id);
        if (it == players_.end()) throw json::ParseError{"NO_PLAYER"};
        return it->second;
    }
    
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
        const auto name = json::getString(parse(body), "name");
        // has value
        // for (const auto& kv : players_)
        // {
        //     if (kv.second == name)
        //     {
        //         return Error{"PLAYER_EXISTS"};
        //     }
        // }
        if (hasValue(players_, name)) return Error{"PLAYER_EXISTS"};
        const auto id = uuid();
        const auto ret = players_.insert({id, name});
        assert(ret.second);
        return json::Json(
        {
            {"success", true},
            {"id", id}
        });
    }
    
    std::string createGame(const std::string& body)
    {
        const auto doc = parse(body);
        const std::string id = json::getString(doc, "id");
        const std::string game = json::getString(doc, "game");

        const auto name = getPlayer(id);
        if (hasItem(joinedGames_, id)) return Error{"ALREADY_JOINED"};
        if (hasItem(games_, game)) return Error{"GAME_EXISTS"};

        joinedGames_.insert({id, game});
        games_.emplace(game, std::make_unique<Game>(game, name));
        return Success{};
    }
    
    std::string joinGame(const std::string& body)
    {
        const auto doc = parse(body);
        const std::string id = json::getString(doc, "id");
        const std::string game = json::getString(doc, "game");

        const auto name = getPlayer(id);
        
        if (hasItem(joinedGames_, id)) return Error{"ALREADY_JOINED"};
        
        const auto git = games_.find(game);
        if (git == games_.end()) return Error{"NO_GAME"};

        joinedGames_.insert({id, game});
        return git->second->addPlayer(name);
    }

    std::string startGame(const std::string& body)
    {
        const auto gp = getGamePlayer(parse(body));

        // Start game and round
        const auto rv = gp.first->startGame();
        return rv ? gp.first->startRound().str() : rv.str();
    }

    std::string startRound(const std::string& body)
    {
        const auto gp = getGamePlayer(parse(body));
        return gp.first->startRound();
    }

    std::string bid(const std::string& body)
    {
        auto doc = parse(body);
        auto gp = getGamePlayer(doc);
        return gp.first->bid(gp.second,
            json::getInt(doc, "n"),
            json::getInt(doc, "face"));
    }

    std::string challenge(const std::string& body)
    {
        auto gp = getGamePlayer(parse(body));
        return gp.first->challenge(gp.second);
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
                    git->second->serialize(w, name);
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
            it.second->serialize(w, "");
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
        //prettyPrint(doc);
        // Players...
        if (doc.IsObject() && doc.HasMember("players") && doc["players"].IsArray())
        {
            for (const auto& el : doc["players"].GetArray())
            {
                const auto id = getString2(el, "id");
                const auto name = getString2(el, "name");
                if (id.empty() || name.empty()) continue;
                
                players_.insert({id, name});
                const auto game = getString2(el, "game");
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
                    games_.emplace(game->name(), std::move(game));
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
    try {
        return impl_->login(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
}

std::string Engine::createGame(const std::string& body)
{
    try {
        return impl_->createGame(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
}

std::string Engine::joinGame(const std::string& body)
{
    try {
        return impl_->joinGame(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
}

std::string Engine::startGame(const std::string& body)
{
    try {
        return impl_->startGame(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
}

std::string Engine::startRound(const std::string& body)
{
    try {
        return impl_->startRound(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
}

std::string Engine::bid(const std::string& body)
{
    try {
        return impl_->bid(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
}

std::string Engine::challenge(const std::string& body)
{
    try {
        return impl_->challenge(body);
    } catch (const json::ParseError& e) {
        return Error{e};
    }
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

