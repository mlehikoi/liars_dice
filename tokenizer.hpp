#pragma once

#include <experimental/string_view>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

namespace dice {

/// Efficient tokenizer for getting tokens from a string. Doesn't have the
/// option of getting empty tokens so this class is not usable for splitting csv
/// files where there can be empty tokens.
class Tokenizer
{
    struct Impl
    {
        template<typename StrType, typename DelimType>
        Impl(StrType&& str, DelimType&& delim)
          : str_{std::forward<StrType>(str)},
            delim_{std::forward<DelimType>(delim)}
        {  
        }
        std::string str_;
        std::string delim_;
    };
    std::unique_ptr<Impl> impl_;
    const std::experimental::string_view str_;
    const std::experimental::string_view delim_;
    std::size_t pos_;

    auto movePos(std::size_t len)
    {
        const auto prev = pos_;
        pos_ += len + 1;
        return prev;
    }
public:
    /// Construct Tokenizer. Tokenizer will store pointers to str and delim.
    /// This version of constructor is cheaper than the one that takes
    /// std::strings.
    /// @param str [in] string to tokenize
    /// @param delim [in] delimiter by which to tokenize
    Tokenizer(const char* str, const char* delim)
    : impl_{},
      str_{str},
      delim_{delim},
      pos_{}
    {
    }

    /// Construct Tokenizer. Tokenizer will make copy of str and delim. If you
    /// want to optimize performance, use the constructor that takes c strings.
    /// @tparm StrType [in] type of the string
    /// @tparam DelimType [in] type of the delimer
    /// @param str [in] string to tokenize
    /// @param delim [in] delimiter by which to tokenize
    template<typename StrType, typename DelimType>
    Tokenizer(StrType&& str, DelimType&& delim)
    : impl_{std::make_unique<Impl>(std::forward<StrType>(str), std::forward<DelimType>(delim))},
      str_{impl_->str_.data()},
      delim_{impl_->delim_.data()},
      pos_{}
    {
    }

    /// Construct Tokenizer where the strings are owned by Tokenizer. Needs
    /// to allocate space for the temporary strings, which carries a small
    /// performance penalty.
    /// @param str [in] string to tokenize
    /// @param delim [in] delimiter by which to tokenize
    //Tokenizer(std::string&& str, std::string&& delim) = 
    //   : impl_{std::make_unique<Impl>(std::move(str), std::move(delim))},
    //     str_{impl_->str_},
    //     delim_{impl_->delim_},
    //     pos_{}
    // {
    //     //std::cout << "RValue" << std::endl;
    // }

    /// Prevent copying
    Tokenizer(Tokenizer&&) = delete;

    /// Get the next token's starting position and length of the token.
    /// @see also getNextString and getNextStringView.
    /// @return pair (pos, len) or {0, 0} if no tokens
    std::pair<std::size_t, std::size_t> next()
    {
        if (pos_ == std::string::npos) return {0, 0};
        // Skip delimiters
        pos_ = str_.find_first_not_of(delim_, pos_);
        if (pos_ == std::string::npos) return {0, 0};
        // Find next delimiters
        auto endPos = str_.find_first_of(delim_, pos_ + 1);
        const std::size_t len = (endPos == std::string::npos) ?
            str_.size() - pos_ : endPos - pos_;
        return std::make_pair(movePos(len), len);
    }

    /// @return next token or empty if no more tokens
    auto nextStringView()
    {
        const auto pl = next();
        return std::experimental::string_view(str_.data() + pl.first, pl.second);
    }

    /// @return next token or empty if no more tokens
    auto nextString()
    {
        const auto pl = next();
        return std::string{str_.data() + pl.first, pl.second};
    }

    /// @return vector of token strings
    auto tokens()
    {
        std::vector<std::string> t;
        for (;;)
        {
            const auto str = nextString();
            if (str.empty()) break;
            t.push_back(std::move(str));
            //t.push_back(str);
        }
        return t;
    }

    /// @return vector of token string_views
    auto tokenViews()
    {
        std::vector<std::experimental::string_view> t;
        for (;;)
        {
            const auto str = nextStringView();
            if (str.empty()) break;
            t.push_back(std::move(str));
        }
        return t;
    }
};

} // namespace dice
