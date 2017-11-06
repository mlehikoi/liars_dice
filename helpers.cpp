#include "helpers.hpp"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <fstream>
#include <sstream>

namespace dice {

std::string slurp(const std::string& path)
{
    std::ifstream f{path, std::ios::binary | std::ios::ate};
    const auto sz = f.tellg();
    if (sz > 0)
    {
        f.seekg(0, std::ios::beg);
        auto buf = std::make_unique<char[]>(std::size_t(sz));
        f.read(buf.get(), sz);
        return std::string(buf.get(), std::size_t(sz));
    }
    return "";
}

void dump(const std::string& path, const std::string& data)
{
    std::ofstream f{path, std::ios::binary};
    f.write(data.data(), std::streamsize(data.size()));
}

std::string uuid()
{
    static auto generator = boost::uuids::random_generator();
    const auto uuid = generator();
    std::stringstream ss;
    ss << uuid;
    return ss.str();
}

std::string getExtension(const std::string& path)
{
    auto dot = path.find_last_of('.');
    if (dot != std::string::npos)
    {
        return path.c_str() + dot + 1;
    }
    return "";
}

std::string getContentType(const std::string& path)
{
    static const std::unordered_map<std::string, std::string> types{
        { "jpg", "image/jpeg" },
        { "png", "image/png" },
        { "js", "application/javascript" },
        { "html", "text/html; charset=utf-8"},
        { "ico", "image/x-icon"},
        { "svg", "image/svg+xml"}
    };
    const auto it = types.find(getExtension(path));
    return it != types.end() ? it->second : "";
}

} // bluff
