#include "brotli.hpp"

#include "atend.hpp"
#include "filehelpers.hpp"

#include <brotli/encode.h>
#include <brotli/decode.h>

#include <cassert>
#include <cstdint>
#include <memory>

namespace dice {

std::string compress(const std::string& orig)
{
    size_t encodedSize = BrotliEncoderMaxCompressedSize(orig.size());
    auto outBuf = std::make_unique<std::uint8_t[]>(encodedSize);
    auto ret = BrotliEncoderCompress(
        BROTLI_DEFAULT_QUALITY,
        BROTLI_DEFAULT_WINDOW,
        BROTLI_MODE_TEXT,
        orig.size(), // Not including terminating zero
        reinterpret_cast<const std::uint8_t*>(orig.data()),
        &encodedSize,
        outBuf.get());
    assert(ret);
    return std::string{reinterpret_cast<const char*>(outBuf.get()), encodedSize};
}

std::string decompress(const std::string& compressed)
{
    std::string result;

    // In
    auto availableIn = compressed.size();
    auto nextIn = reinterpret_cast<const std::uint8_t*>(compressed.data());
    
    // Out
    std::uint8_t outBuf[128];
    std::size_t availableOut = sizeof(outBuf);
    auto nextOut = outBuf;
    std::size_t totalOut = 0;

    auto* s = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    AtEnd ae{[s]{ BrotliDecoderDestroyInstance(s); }};
    for (;;)
    {
        const auto ret = BrotliDecoderDecompressStream(
            s, &availableIn, &nextIn, &availableOut, &nextOut, &totalOut);
        switch (ret)
        {
        case BROTLI_DECODER_RESULT_SUCCESS:
            return result.append(reinterpret_cast<const char*>(outBuf), totalOut - result.size());
        case BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT:
            result.append(reinterpret_cast<const char*>(outBuf), totalOut - result.size());
            nextOut = outBuf;
            availableOut = sizeof(outBuf);
            break;
        case BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT:
        case BROTLI_DECODER_RESULT_ERROR:
            throw std::runtime_error("INVALID_FORMAT");
        }
    }
}

} // namespace dice
