#pragma once
#include "json.hpp"

#include <vector>

namespace dice {

constexpr int STAR = 6;

/**
 * Represents a bid. The bid consist of a selected face and
 * number of dice with that face. For example a bid could be
 * - 2 x 5 (2 fives) or
 * - 3 x * (3 stars)
 */
class Bid
{
    int n_;
    int face_;
public:
    /**
     * Construct Bid. Default to n = 0, face = 0.
     */
    Bid();

    /**
     * Construct Bid.
     * @param n [in] number of dice with given face
     * @param face [in] the dice face for the bid
     */
    Bid(int n, int face);
    
    /** @return number of face in the bid */
    auto n() const { return n_; }

    /** @return the face in the bid */
    auto face() const { return face_; }

    int score() const;
    bool valid() const { return score() > 0; }

    /// Equality operator
    bool operator==(const Bid& bid) const { return n_ == bid.n_ && face_ == bid.face_; }
    /// Less than
    bool operator<(const Bid& other) const;
    /// Less than equal
    bool operator<=(const Bid& other) const;
    /// Greater than
    bool operator>(const Bid& other) const;
    /// Greater than equal
    bool operator>=(const Bid& other) const;

    /**
      * @return bid's difference to actual
      * - < 0: fewer than bid
      * - 0: exactly right amount
      * - > 0: more than bid
    */
    int challenge(const std::vector<int>& commonHand) const;

    /**
     * Serialize bid to give writer.
     * @param w [out] where to serialize
     */
    void serialize(json::Writer& w) const;

    /**
     * Construct bid object from given json.
     * @param v [in] json value to parse
     * @throws ParseError if json format is unexpected
     */
    static Bid fromJson(const rapidjson::Value& v);
};

} // namespace dice
