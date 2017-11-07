#pragma once

#include <string>
#include <iostream>

#include <regex>

namespace dice {

namespace detail {
inline bool breakAfter(const std::string& contents, std::size_t pos, const std::size_t len)
{
    if (pos + len == contents.size()) return true;
    const auto nchar = contents[pos + len];
    return nchar == ',' || nchar == ' ' || nchar == ';';
}

inline bool breakBefore(const std::string& contents, std::size_t pos)
{
    if (pos == 0) return true;
    const auto pchar = contents[pos - 1];
    return pchar == ' ' || pchar == ',';
}

} // namespace detail

/// Check if the http header field contains the given value. E.g.,
/// If the http header field is Accept-Encoding: gzip, br
/// this method would return true to gzip and br and false to b or brz.
/// @param contents [in] contents where to search. Note, this is without
///     the name of the http header field.
/// @param value [in] value to search for
/// @return whether the search value was found
inline bool hasHttpValue(const std::string& contents, const std::string& value)
{
    if (value.empty()) return false;
    const auto pos = contents.find(value);
    return pos == std::string::npos ? false :
        detail::breakBefore(contents, pos) && detail::breakAfter(contents, pos, value.size());
}
} // namespace dice
