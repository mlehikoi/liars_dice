#pragma once

#include "tokenizer.hpp"

#include <string>

namespace dice {

/// Check if the http header field contains the given value. E.g.,
/// If the http header field is Accept-Encoding: gzip, br
/// this method would return true to gzip and br and false to b or brz.
/// @param contents [in] contents where to search. Note, this is without
///     the name of the http header field.
/// @param value [in] value to search for
/// @return whether the search value was found
inline bool hasHttpValue(const std::string& contents, const std::string& value)
{
    static const std::string delim{" ,;"};
    Tokenizer tok(contents.c_str(), delim.c_str());
    for (;;)
    {
        const auto str = tok.nextStringView();
        if (str.empty()) return false;
        if (str == value) return true;
    }
    return true;
}

} // namespace dice
