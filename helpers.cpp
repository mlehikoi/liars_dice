#include "helpers.hpp"

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <fstream>
#include <sstream>

namespace dice {

std::string uuid()
{
    static auto generator = boost::uuids::random_generator();
    const auto uuid = generator();
    std::stringstream ss;
    ss << uuid;
    return ss.str();
}

} // bluff
