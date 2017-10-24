#include "crow.h"
//#include "crow_all.h"

#include "json.hpp"
#include "helpers.hpp"
#include "engine.hpp"

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
    static dice::Engine engine{"db.json"};
    
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]{ return readFile("index.html"); });
    
    //@TODO This isn't login but register
    CROW_ROUTE(app, "/api/login")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            return engine.login(req.body);
        }
    );
    
    CROW_ROUTE(app, "/api/status")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            return engine.status(req.body);
        }
    );
    
    CROW_ROUTE(app, "/api/newGame")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            return engine.createGame(req.body);
        }
    );

    CROW_ROUTE(app, "/api/join")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            return engine.joinGame(req.body);
        }
    );

    CROW_ROUTE(app, "/api/startGame")
    .methods("POST"_method)
    ([](const crow::request& req)
    {
        auto r = engine.startGame(req.body);
        std::cout << "Return " << r << endl;
        return r;
    }
);

    CROW_ROUTE(app, "/api/games")([]{
        const auto data = engine.getGames();
        return data;
        // crow::response r{data};
        // r.add_header("Content-Type", "application/json; charset=utf-8");
        // return r;
    });

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