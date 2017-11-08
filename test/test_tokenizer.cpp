#include "tokenizer.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std;

using PL = std::pair<std::size_t, std::size_t>;

TEST(TokenizerTest, Next) {
    dice::Tokenizer tok{"a,bc,def", ","};
    ASSERT_EQ(tok.next(), PL(0, 1));
    ASSERT_EQ(tok.next(), PL(2, 2));
    ASSERT_EQ(tok.next(), PL(5, 3));
    ASSERT_EQ(tok.next(), PL(0, 0));
    ASSERT_EQ(tok.next(), PL(0, 0));
}

TEST(TokenizerTest, NextStringView) {
    dice::Tokenizer tok{"a,bc,def", ","};
    ASSERT_EQ(tok.nextStringView(), "a");
    ASSERT_EQ(tok.nextStringView(), "bc");
    ASSERT_EQ(tok.nextStringView(), "def");
    ASSERT_EQ(tok.nextStringView(), "");
}

TEST(TokenizerTest, NextString) {
    dice::Tokenizer tok{"a,bc,def", ","};
    ASSERT_EQ(tok.nextString(), "a");
    ASSERT_EQ(tok.nextString(), "bc");
    ASSERT_EQ(tok.nextString(), "def");
    ASSERT_EQ(tok.nextString(), "");
}

TEST(TokenizerTest, Tokens) {
    dice::Tokenizer tok{"a,bc,,,def", ","};
    const auto tokens = tok.tokens();
    ASSERT_EQ(3, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("bc", tokens[1]);
    ASSERT_EQ("def", tokens[2]);
}

TEST(TokenizerTest, TokenViews) {
    dice::Tokenizer tok{"a,bc,,,def", ","};
    const auto tokens = tok.tokenViews();
    ASSERT_EQ(3, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("bc", tokens[1]);
    ASSERT_EQ("def", tokens[2]);
}

TEST(TokenizerTest, LValue1) {
    const std::string str{"a"};
    dice::Tokenizer tok{str, ","};
    const auto tokens = tok.tokens();
    ASSERT_EQ(1, tokens.size());
    ASSERT_EQ("a", tokens[0]);
}

TEST(TokenizerTest, LValue2) {
    const std::string delim{","};
    dice::Tokenizer tok{"a", delim};
    const auto tokens = tok.tokens();
    ASSERT_EQ(1, tokens.size());
    ASSERT_EQ("a", tokens[0]);
}

TEST(TokenizerTest, MultipleDelimiters) {
    dice::Tokenizer tok{" a, b,c;;", " ,;"};
    const auto tokens = tok.tokens();
    ASSERT_EQ(3, tokens.size());
    ASSERT_EQ("a", tokens[0]);
    ASSERT_EQ("b", tokens[1]);
    ASSERT_EQ("c", tokens[2]);
}

} // Unnamed namespace