//#include "crow.h"
//#include "crow_all.h"

#include "json.hpp"
#include "helpers.hpp"

#include <string>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.


using namespace std;
using namespace dice;

namespace dice {
auto readFile(const std::string& name)
{
    const auto path = "../static/"s + name;
    const auto data = dice::slurp("../static/"s + path);
    if (data.empty()) return crow::response(404);
    crow::response r{data};
    r.add_header("Content-Type", getContentType(name));
    return r;
}
} // dice

class Games
{
public:
    std::unordered_map<std::string, int> map_;

    void addGame(const std::string& id)
    {
        map_.insert({id, 0});
        std::cout << map_.size() << std::endl;
    }
};

auto& games()
{
    static Games games_;
    return games_;
}
struct PlayerInfo
{
    std::string name_;
    std::string game_;
};
static std::unordered_map<std::string, PlayerInfo> players_;
static std::unordered_set<std::string> games_;

int main()
{
    std::ifstream is("players.dat");
    while (is.good())
    {
        std::string id, name, game;
        is >> id >> name >> game;
        players_[id] = {name, game};
    }
    
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]{ return readFile("index.html"); });
    
    CROW_ROUTE(app, "/api/login")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            auto j = crow::json::load(req.body);
            const std::string name = j["name"].s();
            for (const auto& idName : players_)
            {
                if (idName.second.name_ == name)
                {
                    return json::Json({"success", false}).str();
                }
            }
            const auto id = uuid();
            players_.insert({id, {name, ""}});
            std::ofstream os("players.dat");
            for (const auto& idName : players_)
                os << idName.first << "\t" << idName.second.name_ << "\t" << idName.second.game_ << endl;
            return json::Json({{"success", true}, {"playerId", id}}).str();
        }
    );
    
    CROW_ROUTE(app, "/api/status")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            crow::json::wvalue response;
            auto j = crow::json::load(req.body);
            if (j["id"].t() == crow::json::type::String)
            {
                const std::string id = j["id"].s();
                const auto it = players_.find(id);
                if (it != players_.end())
                {
                    response["success"] = true;
                    response["playerId"] = id;
                    response["name"] = it->second.name_;
                    response["game"] = it->second.game_;
                    return crow::response{response};
                }
            }
            response["success"] = false;
            return crow::response(response);
        }
    );
    
    CROW_ROUTE(app, "/api/newGame")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            crow::json::wvalue response;
            auto j = crow::json::load(req.body);
            cout << req.body << endl;
            const std::string id = j["id"].s();
            const std::string game = j["game"].s();
            if (games_.insert(game).second)
            {
                response["success"] = true;
                return response;
            }
            response["success"] = false;
            return response;
        }
    );

    CROW_ROUTE(app, "/api/list")([]{
        std::stringstream ss;
        ss << "Games" << std::endl;
        for (const auto& game : games().map_)
        {
            ss << "..." << game.first << std::endl;
        }
        return ss.str();
    });

    CROW_ROUTE(app, "/<string>")(readFile);
    
    app.port(8000).run();
}