#pragma once

#include <functional>

namespace dice {

class AtEnd
{
    std::function<void()> f_;
public:
    AtEnd(const std::function<void()>& f) : f_{f} {}
    AtEnd(AtEnd&&) = delete;
    ~AtEnd() { f_(); }
};

} // namespace dice
