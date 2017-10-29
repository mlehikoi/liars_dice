#pragma once
#include "helpers.hpp"

#include <vector>

namespace dice {

const int STAR = 6;

class Bid
{
    int n_;
    int face_;
public:
    Bid();
    Bid(int n, int face);
    
    auto n() const { return n_; }
    auto face() const { return face_; }

    int score() const;
    bool valid() const { return score() > 0; }
    bool operator<(const Bid& other) const;
    bool operator<=(const Bid& other) const;
    bool operator>(const Bid& other) const;
    bool operator>=(const Bid& other) const;

    /**
      * @return bid's difference to actual
      * < 0: fewer than bid
      *   0: exactly right amount
      * > 0: more than bid
    */
    int challenge(const std::vector<int>& commonHand) const;
    void serialize(Writer& w) const;
    static Bid fromJson(const rapidjson::Value& v);
    bool operator==(const Bid& bid) const { return n_ == bid.n_ && face_ == bid.face_; }
};

} // namespace dice
