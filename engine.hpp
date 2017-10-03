#pragma once
#include <memory>
#include <utility>
#include <string>

namespace dice {

class Engine
{
public:
    Engine(const char* filename);
    ~Engine();
    std::string login(const std::string& body);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace dice
