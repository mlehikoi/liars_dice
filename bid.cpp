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
    if (n_ <= 0)
    {
        return 0;
    }
    else if (face_ == STAR)
    {
        return 6 + (n_ - 1) * 11;
    }
    // Leave "space" for stars
    const int stars = n_ / 2;
    return (n_ - 1) * 5 + face_ + stars;
}

Bid Bid::fromScore(int score)
{
    if (score == 0)
    {
        return {0, 0};
    }
    const auto stars = std::div(score + 5, 11);
    if (stars.rem == 0) // star
    {
        return {stars.quot, STAR};
    }
    const auto dice = std::div(score - stars.quot - 1, 5);
    return {dice.quot + 1, dice.rem + 1};
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
