//#include "crow.h"
//#include "crow_all.h"

#include <string>
#include <sstream>
#include <unordered_map>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include "helpers.hpp"

using namespace std;
using namespace bluff;

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

static std::unordered_map<std::string, std::string> players_;

int main()
{
    std::ifstream is("players.dat");
    while (is.good())
    {
        std::string id, name;
        is >> id >> name;
        players_[id] = name;
    }
    
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });
    
    CROW_ROUTE(app, "/api/login")
        .methods("POST"_method)
        ([](const crow::request& req)
        {
            crow::json::wvalue response;
            auto j = crow::json::load(req.body);
            const std::string name = j["name"].s();
            for (const auto& idName : players_)
            {
                if (idName.second == name)
                {
                    response["success"] = false;
                    return response;
                }
            }
            const auto id = uuid();
            players_.insert({id, name});
            std::ofstream os("players.dat");
            for (const auto& idName : players_)
                os << idName.first << "\t" << idName.second << endl;
            response["success"] = true;
            response["playerId"] = id;
            return response;
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
                    response["name"] = it->second;
                    return crow::response{response};
                }
            }
            response["success"] = false;
            return crow::response(response);
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

    CROW_ROUTE(app, "/<string>")(
        [](std::string path)
    {
        cout << "Path: " << path << endl;
        auto data = slurp("../static/"s + path);
        crow::response r{data};
        r.add_header("Content-Type", getContentType(data));
        return r;
    });
    
    //app.port(18080).multithreaded().run();
    app.port(8000).run();
}