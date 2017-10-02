#include "engine.hpp"

#include "helpers.hpp"

namespace dice {

std::pair<std::string, std::string> readFile(const std::string& base, const std::string& name)
{
    const auto path = dice::slurp(base + "/static/" + name);
    auto data = dice::slurp(path);
    //cout << data.size() << endl;
    return std::make_pair(getContentType(path), path);
    //r.add_header("Content-Type", );
    //return r;
}

} // namespace dice

