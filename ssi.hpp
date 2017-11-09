#pragma once
#include <string>

namespace dice {
/// Read html file from the given location and replace include statements with
/// the actual file where include points to. Note! this function does not
/// recursively add includes to the included files.
/// @param name [in] name of the file
/// @param base [in] directory for the html file and includes
/// @return contents of html file with includes
std::string readHtml(const std::string& name, const std::string& base);

} // namespace dice
