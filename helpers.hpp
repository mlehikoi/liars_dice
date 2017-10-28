#pragma once

#include "json.hpp"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

namespace dice {

using Writer = rapidjson::PrettyWriter<rapidjson::StringBuffer>;

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

template<typename Doc>
int getInt(const Doc& doc, const char* member)
{
    return doc.IsObject() && doc.HasMember(member) && doc[member].IsInt() ?
        doc[member].GetInt() : -1;
}

class RetVal
{
    std::string str_;
    const bool good_;
public:
    RetVal(const std::string& str, bool good) : str_{str}, good_{good} {}
    auto str() const { return str_; }
    operator std::string() const { return str(); }
    operator bool() const { return good_; }
};

class Error : public RetVal
{
public:
    Error(const char* msg)
      : RetVal{json::Json({
            {"success", false},
            {"error", msg}
        }).str(),
        false}
    {
    }
};

class Success : public RetVal
{
public:
    Success() : RetVal{"{\"success\": true}", true} {}
};

// Enum class
template<typename T>
constexpr inline std::size_t enumSize()
{
    return enumSize(T{});
}

template<typename T>
constexpr inline const std::map<T, std::string>& getEnums()
{
    return getEnums(T{});
}

inline std::string consumeToken(const char*& rpStrP)
{
    std::string tmp;
    for (; *rpStrP; ++rpStrP)
    {
        if (*rpStrP == ' ') continue;
        else if (*rpStrP == ',') { ++rpStrP; break; }
        else tmp += *rpStrP;
    }
    return tmp;
}

template<typename Arg>
inline void createEnumMap(std::map<Arg, std::string>& rMapP, const char* names, Arg&& arg)
{
    rMapP[std::forward<Arg>(arg)] = consumeToken(names);
    // We should have consumed all the strings by now
    assert(!*names);
}

template<typename Arg, typename... Args>
inline void createEnumMap(std::map<Arg, std::string>& rMapP, const char* names, Arg&& arg, Args&&... args)
{
    rMapP[std::forward<Arg>(arg)] = consumeToken(names);
    createEnumMap(rMapP, names, std::forward<Args>(args)...);
}

template<typename Arg, typename... Args>
inline auto createEnumMap(const char* names, Arg&& arg, Args&&... args)
{
    std::map<Arg, std::string> map;
    createEnumMap(map, names, std::forward<Arg>(arg), std::forward<Args>(args)...);
    return map;
}

template<typename T>
const char* findFromMap(T key)
{
    auto& map = getEnums(key);
    const auto it = map.find(key);
    return it != map.end() ? it->second.c_str() : "Unknown";
}

template<typename T, size_t SIZE>
constexpr inline int numElements(const T (&)[SIZE])
{
    return SIZE;
}

template<typename Arg, typename... Args>
constexpr inline Arg firstParam(Arg&& arg, Args...)
{
    return std::forward<Arg>(arg);
}
} // dice

#define MAKE_ENUM(Name, ...) enum Name {__VA_ARGS__}; \
constexpr std::size_t enumSize(Name) { return ::dice::numElements({__VA_ARGS__}); }\
inline static const auto& getEnums(Name) { \
    static const auto enums = ::dice::createEnumMap(#__VA_ARGS__, __VA_ARGS__); \
    return enums;\
}\
/*static struct Init ## Name { Init ## Name () { getEnums(::dice::firstParam(__VA_ARGS__));} } init ## Name ## G;*/\
inline const char* toString(Name key) const { \
    auto& map = getEnums(key); \
    const auto it = map.find(key); \
    return it != map.end() ? it->second.c_str() : "Unknown"; \
} //{ return ::dice::findFromMap(n); }

namespace dice {
std::string slurp(const std::string& path);
void dump(const std::string& path, const std::string& data);

std::string uuid();

std::string getExtension(const std::string& path);

std::string getContentType(const std::string& path);

inline auto parse(const std::string& json)
{
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    return doc;
}

template<typename Doc>
inline void prettyPrint(Doc& doc)
{
    rapidjson::StringBuffer s;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> w{s};
    doc.Accept(w);
    std::cout << s.GetString() << std::endl;
}

} // namespace dice
