#pragma once
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

const int STAR = 6;
class Bid
{
    int n_;
    int face_;
public:
    Bid() : n_{}, face_{} {}
    Bid(int n, int face) : n_{n}, face_{face} {}
    
    auto n() const { return n_; }
    auto face() const { return face_; }
    auto score() const
    {
        return face_ == STAR ? n_ * 20 : n_ * 10 + face_;
    }
    bool operator<(const Bid& other) const
    {
        return score() < other.score();
    }
    
    /**
      * @return bid's difference to actual
      * < 0: fewer than bid
      *   0: exactly right amount
      * > 0: more than bid
    */
    int challenge(const std::vector<int>& commonHand) const
    {
        int n = 0;
        for (auto d : commonHand)
        {
            
            if (d == STAR) ++n;
            else if (d == face_) ++n;
        }
        return n - n_;
    }

    void serialize(rapidjson::PrettyWriter<rapidjson::StringBuffer>& w) const
    {
        w.StartObject();
        w.Key("n"); w.Int(n_);
        w.Key("face"); w.Int(face_);
        w.EndObject();
    }

    static auto fromJson(const rapidjson::Value& v)
    {
        if (v.IsObject() &&
            v.HasMember("n") && v["n"].IsInt() &&
            v.HasMember("face") && v["face"].IsInt())
        {
            return Bid{v["n"].GetInt(), v["face"].GetInt()};
        }
        return Bid{};
    }

    const bool operator==(const Bid& bid) const { return n_ == bid.n_ && face_ == bid.face_; }
};