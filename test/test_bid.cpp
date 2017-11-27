#include "bid.hpp"

#include "gtest/gtest.h"

using namespace std;

namespace {
//using Dice::Bid;
struct BidScore
{
    dice::Bid bid;
    int score;
};
BidScore scores[] =
{
    { {0, 0}, 0 },
    { {1, 1}, 1 },
    { {1, 2}, 2 },
    { {1, 3}, 3 },
    { {1, 4}, 4 },
    { {1, 5}, 5 },

    { {1, 6}, 6 },

    { {2, 1}, 7 },
    { {2, 2}, 8 },
    { {2, 3}, 9 },
    { {2, 4}, 10 },
    { {2, 5}, 11 },
    { {3, 1}, 12 },
    { {3, 2}, 13 },
    { {3, 3}, 14 },
    { {3, 4}, 15 },
    { {3, 5}, 16 },

    { {2, 6}, 17 },

    { {4, 1}, 18 },
    { {4, 2}, 19 },
    { {4, 3}, 20 },
    { {4, 4}, 21 },
    { {4, 5}, 22 },
    { {5, 1}, 23 },
    { {5, 2}, 24 },
    { {5, 3}, 25 },
    { {5, 4}, 26 },
    { {5, 5}, 27 },

    { {3, 6}, 28 },

    { {6, 1}, 29 },
    { {6, 2}, 30 },
    { {6, 3}, 31 },
    { {6, 4}, 32 },
    { {6, 5}, 33 },
    { {7, 1}, 34 },
    { {7, 2}, 35 },
    { {7, 3}, 36 },
    { {7, 4}, 37 },
    { {7, 5}, 38 },

    { {4, 6}, 39 },
    
    { {8, 1}, 40 }
};

TEST(BidTest, ToScore) {
    for (const auto& score : scores)
    {
        //cout << score.bid.n() << " " << score.bid.face() << endl;
        ASSERT_EQ(score.score, score.bid.score());
    }
}

TEST(BidTest, FromScore) {
    for (const auto& score : scores)
    {
        const auto bid = dice::Bid::fromScore(score.score);
        ASSERT_EQ(score.bid.face(), bid.face());
        ASSERT_EQ(score.bid.n(), bid.n());
    }
}

} // unnamed namespace
