#pragma once

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>
#include <string>

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