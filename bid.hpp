#pragma once

const int STAR = 6;
class Bid
{
    int n_;
    int face_;
public:
    Bid() : n_{}, face_{} {}
    Bid(int n, int face) : n_{n}, face_{face} {}
    
    auto score() const
    {
        return face_ == STAR ? n_ * 20 : n_ * 10 + face_;
    }
    bool operator<(const Bid& other) const
    {
        return score() < other.score();
    }
};