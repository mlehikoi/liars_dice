#include "brotli.hpp"

#include "filehelpers.hpp"

#include "gtest/gtest.h"

namespace {

using namespace std;

TEST(BrotliTest, EncodeDecode) {
    const auto orig = dice::slurp("../final.json");
    const auto deflated = dice::compress(orig);
    ASSERT_LE(deflated.size(), orig.size());
    
    const auto inflated = dice::decompress(deflated);
    ASSERT_EQ(inflated.size(), orig.size());
    ASSERT_EQ(orig, inflated);
}

} // Unnamed namespace
