#pragma once

#include <stdexcept>
#include <string_view>
#include <algorithm>



template<size_t max_length>
struct FixedString {
  constexpr FixedString(const char* string, size_t length)
  : size_(length)
  {
    if (length > max_length)
      throw std::logic_error("length must be < max_length");
    std::ranges::copy(string, string + length, data_);
    std::ranges::fill(data_ + length, data_ + max_length, '\0');
  }
  constexpr operator std::string_view() const {
    return std::string_view(data_, size_);
  }

  constexpr void push_back(char value) {
    if (size_ == max_length - 1) {
      throw std::logic_error("length must be < max_length");
    }
    data_[size_] = value;
    ++size_;
  }

  char data_[max_length];
  size_t size_;

  template<char... Literal>
  friend consteval FixedString<sizeof...(Literal)> operator ""_cstr();

  constexpr FixedString(char const (&string)[max_length])
  : size_(max_length)
  {
    std::ranges::copy(string, string + max_length, data_);
  }
};

template<FixedString A>
consteval auto operator""_cstr()
{
    return FixedString<256>(A.data_, A.size_ - 1);
}
