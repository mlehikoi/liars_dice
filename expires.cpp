#include "expires.hpp"

#include <chrono>       // std::chrono::system_clock
#include <ctime>        // std::time_t, struct std::tm, std::gmtime
#include <iomanip>      // std::put_time
#include <sstream>      // stringstream

namespace dice {

std::string expires()
{
    const auto expires = std::chrono::system_clock::now() + std::chrono::hours{365 * 24};
    std::time_t tt = std::chrono::system_clock::to_time_t(expires);
    std::tm * ptm = std::gmtime(&tt);

    std::stringstream ss;
    // Need to make sure that this works alse when default locale is non-English
    ss.imbue(std::locale{"en_US"});
    // GMT needs to be hard-coded as specified in RFC1123
    ss << std::put_time(ptm, "%a, %d %b %Y %H:%M:%S GMT");
    return ss.str();
}

} // namespace dice
