#include "helpers.hpp"

#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>

#pragma once

namespace {

class AtEnd
{
    std::function<void()> f_;
public:
    AtEnd(const std::function<void()>& f) : f_{f} {}
    AtEnd(AtEnd&&) = delete;
    ~AtEnd() { f_(); }
};

inline std::string tmpName(const char* prefix)
{
    char tmp[4096];
    std::strcpy(tmp, prefix);
    std::strcat(tmp, "XXXXXX");
    auto f = ::mkstemp(tmp);
    ::close(f);
    return tmp;
}

class TmpFile
{
    const std::string filename_;
public:
    TmpFile(const std::string& filename) : filename_{filename} {}
    ~TmpFile() { std::remove(filename_.c_str()); }
    TmpFile(TmpFile&& other) : filename_{other.filename_} {}
    auto str() { return filename_; }
};

inline auto tmpCopy(const char* src, const char* targetPrefix)
{
    auto tmp = tmpName(targetPrefix);
    dice::dump(tmp, dice::slurp(src));
    return TmpFile{tmp};
}

inline bool fileExists(const std::string& filename) 
{
    struct stat fileInfo;
    return ::stat(filename.c_str(), &fileInfo) == 0;
}

} // Unnamed namespace