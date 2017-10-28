#include "bid.hpp"

namespace dice {

int Bid::score() const
{
    return face_ == STAR ? n_ * 20 : n_ * 10 + face_;
}
bool Bid::operator<(const Bid& other) const
{
    return score() < other.score();
}

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

void Bid::serialize(Writer& w) const
{
    w.StartObject();
    w.Key("n"); w.Int(n_);
    w.Key("face"); w.Int(face_);
    w.EndObject();
}

Bid Bid::fromJson(const rapidjson::Value& v)
{
    if (v.IsObject() &&
        v.HasMember("n") && v["n"].IsInt() &&
        v.HasMember("face") && v["face"].IsInt())
    {
        return Bid{v["n"].GetInt(), v["face"].GetInt()};
    }
    return Bid{};
}

} // namespace dice
