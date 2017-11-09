//#pragma once
#include <string>

#include <iostream>
#include <regex>

using namespace std;

namespace dice {
//<!--#include virtual="../quote.txt" -->



}

int main()
{
    std::regex re{R"#(<!--#include virtual="([^"]*)" -->)#", std::regex::optimize};
    //std::regex re{R"#(.)#"};//, std::regex::optimize};
    const std::string str = R"(
        
        
        
        asdaf<!--#include virtual="../quote.txt" -->asdaf
        \n\n\n
        
        #)";
    cout << str << endl;

    std::smatch m;
    std::string str2 = "ABC";
    bool r = std::regex_search(str, m, re);
    cout << r << " " << m.size() << endl;
    for (const auto& match : m)
    {
        cout << match << endl;
    }
    return 0;
}

