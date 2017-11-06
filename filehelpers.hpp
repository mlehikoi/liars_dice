#pragma once
#include <string>

namespace dice {
std::string slurp(const std::string& path);

void dump(const std::string& path, const std::string& data);

std::string getExtension(const std::string& path);

std::string getContentType(const std::string& path);
}
