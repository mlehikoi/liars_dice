#include <crow/app.h>

#include "brotli.hpp"
#include "filehelpers.hpp"
#include "engine.hpp"
#include "expires.hpp"
#include "httphelpers.hpp"
#include "ssi.hpp"

#include <string>

using namespace std;
using namespace dice;

namespace dice {

/**
 * Read the given file
 * @param name [in] filename
 * @return crow response
 */
inline auto readFile(const std::string& name)
{
    const auto path = "../static/"s + name;
    std::cout << "Slurping " << path << std::endl;
    const auto data = dice::slurp(path);
    if (data.empty()) return crow::response(404);
    crow::response r{data};
    r.add_header("Content-Type", getContentType(name));
    r.add_header("Expires", expires());
    return r;
}

inline auto readHtmlFile(const std::string& name)
{
    const auto data = dice::readHtml(name, "../static");
    if (data.empty()) return crow::response(404);
    crow::response r{data};
    r.add_header("Content-Type", getContentType(name));
    r.add_header("Expires", expires());
    return r;
}

inline auto packJson(const std::string& json, const crow::request& req)
{
    crow::response resp;
    if (dice::hasHttpValue(req.get_header_value("Accept-Encoding"), "br"))
    {
        resp.write(dice::compress(json));
        resp.add_header("Content-Encoding", "br");
    }
    else
    {
        resp.write(json);
    }
    resp.add_header("Content-Type", "application/json");
    return resp;
}
} // dice

int main()
{
    static dice::Engine engine{"db.json"};
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]{ return readHtmlFile("index.html"); });

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
            return packJson(engine.status(req.body), req);
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
    });

    CROW_ROUTE(app, "/api/startRound")
    .methods("POST"_method)
    ([](const crow::request& req)
    {
        auto r = engine.startRound(req.body);
        std::cout << "Return " << r << endl;
        return r;
    });

    CROW_ROUTE(app, "/api/bid")
    .methods("POST"_method)
    ([](const crow::request& req)
    {
        auto r = engine.bid(req.body);
        std::cout << "Return " << r << endl;
        return r;
    });

    CROW_ROUTE(app, "/api/challenge")
    .methods("POST"_method)
    ([](const crow::request& req) {
        auto r = engine.challenge(req.body);
        std::cout << "Return " << r << endl;
        return r;
    });

    CROW_ROUTE(app, "/api/games")([] {
        return engine.getGames();
    });

    CROW_ROUTE(app, "/<string>")([](std::string name) {
	return readHtmlFile(name);
    });

    CROW_ROUTE(app, "/<string>/<string>")([](std::string dir, std::string name) {
        return readFile(dir + "/" + name);
    });

    //crow::logger::setLogLevel(crow::LogLevel::CRITICAL);
    app.port(8000).run();
}
