#pragma once
#include <utility>
#include <string>

namespace dice {

std::pair<std::string, std::string> readFile(const std::string& base, const std::string& name);

} // namespace dice
