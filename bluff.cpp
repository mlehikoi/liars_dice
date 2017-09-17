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
int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        return "Hello world";
    });
    
    CROW_ROUTE(app, "/api/create")([]{
        static auto generator = boost::uuids::random_generator();
        const auto uuid = generator();
        std::stringstream ss;
        ss << uuid;
        games().addGame(ss.str());
        ss << " " << games().map_.size();
        return ss.str();
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

    CROW_ROUTE(app, "/<string>")(
        [](std::string path)
    {
        auto data = slurp("../static/"s + path);
        crow::response r{data};
        r.add_header("Content-Type", getContentType(data));
        return r;
    });
    
    //app.port(18080).multithreaded().run();
    app.port(18080).run();
}