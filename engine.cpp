#include "engine.hpp"

#include "helpers.hpp"
#include "json.hpp"
#include <rapidjson/document.h>

#include "rapidjson/reader.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/error/en.h"

using namespace rapidjson;

namespace dice {

struct PlayerInfo
{
    std::string name_;
    std::string game_;
};
static std::unordered_map<std::string, PlayerInfo> players_;
    
class Engine::Impl
{
    const char* filename_;
    rapidjson::Document doc_;
public:
    Impl(const char* filename)
      : filename_{filename},
        doc_{}
    {
        doc_.Parse(filename_);
    }
    
    std::string add(const std::string& name)
    {
        std::cout << "Name: " << name << std::endl;
        if (!doc_.IsObject()) doc_.SetObject();
        if (!doc_.HasMember("players"))
        {
            rapidjson::Value v;
            v.SetObject();
            auto& o = doc_.AddMember("players", v, doc_.GetAllocator());
            
            rapidjson::Value k;
            const auto id = uuid();
            k.SetString(name.c_str(), name.length(), doc_.GetAllocator());
            v.SetString(id.c_str(), id.length(), doc_.GetAllocator());
            o.AddMember(k, v, doc_.GetAllocator());
        }
        
        // Printout
        std::cout << "<pretty>" << std::endl;
        char writeBuffer[65536];
        FileWriteStream os(stdout, writeBuffer, sizeof(writeBuffer));
        PrettyWriter<FileWriteStream> writer(os);
        doc_.Accept(writer);
        std::cout << "</pretty>" << std::endl;
        
        return "";
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
    std::cout << "body " << body << std::endl;
    if (doc.IsObject() && doc.HasMember("name") && doc["name"].IsString())
    {
        const auto id = impl_->add(doc["name"].GetString());
        if (id.empty())
        {
            return json::Json({"success", false}).str();
        }
        return json::Json({{"success", true}, {"playerId", id}}).str(); 
    }
    // auto j = crow::json::load(req.body);
//     const std::string name = j["name"].s();
//     for (const auto& idName : players_)
//     {
//         if (idName.second.name_ == name)
//         {
//             return json::Json({"success", false}).str();
//         }
//     }
//     const auto id = uuid();
//     players_.insert({id, {name, ""}});
//     std::ofstream os("players.dat");
//     for (const auto& idName : players_)
//         os << idName.first << "\t" << idName.second.name_ << "\t" << idName.second.game_ << endl;
//     return json::Json({{"success", true}, {"playerId", id}}).str();
    return "";
}

} // namespace dice

