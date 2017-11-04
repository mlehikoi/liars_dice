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

class LogicError : public std::runtime_error
{
public:
    LogicError(const std::string& what) : std::runtime_error{what} {}
};

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
    auto getGamePlayer(const rapidjson::Value& doc) const
    {
        const std::string id = json::getString(doc, "id");
        
        const auto pit = players_.find(id);
        if (pit == players_.end()) throw LogicError{"NO_PLAYER"};
        
        const auto jit = joinedGames_.find(id);
        if (jit == joinedGames_.end()) throw LogicError{"NOT_JOINED"};
        
        const auto git = games_.find(jit->second);
        assert(git != games_.end());

        return std::make_pair(git->second.get(), pit->second);
    }

    // Get the player for the given doc["id"]
    const auto& getPlayer(const std::string& id) const
    {
        const auto it = players_.find(id);
        if (it == players_.end()) throw LogicError{"NO_PLAYER"};
        return it->second;
    }

    std::string getId(const std::string& name) const
    {
        for (auto kv : players_)
        {
            if (kv.second == name)
            {
                return kv.first;
            }
        }
        return "";
    }

    const Game* getJoinedGame(const std::string& id) const
    {
        const auto jit = joinedGames_.find(id);
        if (jit != joinedGames_.end())
        {
            const auto git = games_.find(jit->second);
            assert(git != games_.end());
            return git->second.get();
        }
        return nullptr;
    }
    
public:
    Impl(const std::string& filename)
      : filename_{filename},
        players_{}
    {
        load();
    }

    std::string login(const std::string& body)
    {
        const auto name = json::getString(parse(body), "name");
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
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        
        json::Object(w, [this, &body](auto& w)
        {
            auto doc = dice::parse(body);
            const std::string id = json::getString(doc, "id");
            const std::string name = getPlayer(id);

            json::KeyValue(w, "success", true);
            json::KeyValue(w, "id", id);
            json::KeyValue(w, "name", name);

            auto game = getJoinedGame(id);
            if (game)
            {
                w.Key("game");
                game->serialize(w, name);
            }
        });
        return s.GetString();
    }
    
    std::string getGames() const
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        
        json::ArrayW(w, [this](auto& w)
        {
            for (const auto& kv : games_)
            {
                kv.second->serializeGameInfo(w);
            }
        });
        return s.GetString();
    }

    void save()
    {
        rapidjson::StringBuffer s;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
        
        json::Object(w, [=](auto& w)
        {
            json::ArrayW(w, "players", [=](auto& w)
            {
                for (const auto& kv : players_)
                {
                    const auto id = kv.first;
                    json::Object(w, [=](auto& w)
                    {
                        json::KeyValue(w, "id", id);
                        json::KeyValue(w, "name", kv.second);
                        
                        const auto it = joinedGames_.find(id);
                        if (it != joinedGames_.end())
                        {
                            json::KeyValue(w, "game", it->second);
                        }
                    });
                }
            });
            json::ArrayW(w, "games", [=](auto& w){
                for (const auto& it : games_)
                {
                    it.second->serialize(w, "");
                }
            });
        });
        
        dump(filename_, s.GetString());
    }

private:
    void readPlayers(const rapidjson::Document& doc)
    {
        for (const auto& p : json::getArray(doc, "players"))
        {
            try {
                players_.emplace(json::getString(p, "id"),
                                 json::getString(p, "name"));
            } catch (const std::exception&) {}
        }
    }

    void readGames(const rapidjson::Document& doc)
    {
        for (const auto& jgame : json::getArray(doc, "games"))
        {
            auto game = Game::fromJson(jgame);
            if (game)
            {
                for (const auto& player : game->players())
                {
                    const auto id = getId(player.name());
                    if (id.empty())
                    {
                        game->removePlayer(player);
                    }
                    else
                    {
                        if (!joinedGames_.emplace(id, game->name()).second)
                        {
                            game->removePlayer(player);
                        }
                    }
                }
                if (!game->players().empty())
                    games_.emplace(game->name(), std::move(game));
            }
        }
    }
    void load() noexcept
    {
        auto doc = parse(slurp(filename_));
        try {
            readPlayers(doc);
            readGames(doc);
        }
        catch (const std::exception&) {}
    }
};

Engine::Engine(const std::string& filename) noexcept
    : impl_{std::make_unique<Impl>(filename)}
{
}

Engine::~Engine() noexcept = default;

std::string Engine::login(const std::string& body) noexcept
{
    try {
        return impl_->login(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::createGame(const std::string& body) noexcept
{
    try {
        return impl_->createGame(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::joinGame(const std::string& body) noexcept
{
    try {
        return impl_->joinGame(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::startGame(const std::string& body) noexcept
{
    try {
        return impl_->startGame(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::startRound(const std::string& body) noexcept
{
    try {
        return impl_->startRound(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::bid(const std::string& body) noexcept
{
    try {
        return impl_->bid(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::challenge(const std::string& body) noexcept
{
    try {
        return impl_->challenge(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::status(const std::string& body) const noexcept
{
    try {
        return impl_->status(body);
    } catch (const std::exception& e) {
        return Error{e.what()};
    }
}

std::string Engine::getGames() const noexcept
{
    return impl_->getGames();
}

void Engine::save() noexcept
{
    impl_->save();
}

} // namespace dice

