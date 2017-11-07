#pragma once

#include <string>

namespace dice {

std::string compress(const std::string& orig);

std::string decompress(const std::string& compressed);

} // namespace dice
