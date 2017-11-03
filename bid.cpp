#include "bid.hpp"

namespace dice {

Bid::Bid()
  : n_{}, face_{}
{
}

Bid::Bid(int n, int face)
  : n_{n}, face_{face}
{
    if (n_ <= 0 || face_ < 1 || face_ > 6)
    {
        n_ = 0;
        face_ = 0;
    }
}

int Bid::score() const
{
    return face_ == STAR ? n_ * 20 : n_ * 10 + face_;
}

bool Bid::operator<(const Bid& other) const
{
    return score() < other.score();
}
bool Bid::operator>(const Bid& other) const { return other < *this; }
bool Bid::operator<=(const Bid& other) const { return !(*this > other); }
bool Bid::operator>=(const Bid& other) const { return !(*this < other); }

int Bid::challenge(const std::vector<int>& commonHand) const
{
    int n = 0;
    for (auto d : commonHand)
    {
        if (d == STAR) ++n;
        else if (d == face_) ++n;
    }
    return n - n_;
}

void Bid::serialize(json::Writer& w) const
{
    json::Json(w,
    {
        {"n", n_},
        {"face", face_}
    });
}

Bid Bid::fromJson(const rapidjson::Value& v)
{
    return Bid{json::getInt(v, "n"),
               json::getInt(v, "face")};
}

} // namespace dice
