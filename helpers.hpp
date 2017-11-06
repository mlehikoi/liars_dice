#pragma once

#include "filehelpers.hpp"
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

/// Check if the given container has the given key.
/// @tparam Container type of container, should have find method
/// @tparam KeyType type of the key in container
/// @param c [in] the container where to find the key
/// @param k [in] the key to search for
/// @return if the key was found
template<typename Container, typename KeyType>
inline bool hasItem(const Container& c, const KeyType& k)
{
    return c.find(k) != c.end();
}

/// Check if the given container has the given value.
/// @tparam Container type of container, should have key-values.
/// Note! the search time is linear.
///
/// @tparam ValueType type of the key in container
/// @param c [in] the container where to find the value
/// @param v [in] the value to search for
/// @return if the valye was found
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
    Error(const std::string& msg)
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

std::string uuid();

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
