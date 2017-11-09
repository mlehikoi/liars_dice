#include "ssi.hpp"

#include <cassert>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

namespace dice {

std::string readHtml(const std::string& name, const std::string& base)
{
    // Benchmarks
    // Baseline: 53 ms
    // Whole string at once: 1630 ms (no replace)
    // Line at time, full solution: 150 ms
    static const std::regex re{R"#(<!--#include virtual\s*=\s*"([^"]*)"\s*-->)#",
        std::regex::optimize};
    std::ifstream src(base + "/" + name);
    std::ostringstream dest{std::ios::binary};
    std::string line;    
    while (std::getline(src, line))
    {
        std::smatch m;
        if (line.find("#include virtual") != std::string::npos &&
            std::regex_search(line, m, re))
        {
            assert(m.size() == 2);
            std::ifstream src2(base + "/" + m.str(1), std::ios::binary);
            if (src2.good()) dest << src2.rdbuf();
        }
        else { dest << line << std::endl; }
    }
    return dest.str();
}

} // namespace dice
