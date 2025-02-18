#pragma once

#include <type_lists.hpp>
#include <type_traits>
#include <value_types.hpp>

namespace detail {
using namespace type_lists;
using namespace value_types;

template <typename T>
using Increment = value_types::ValueTag<T::Value + 1>;

template <typename T>
using NextFib =
    std::pair<typename T::second_type,
              ValueTag<T::second_type::Value + T::first_type::Value>>;

template <typename T>
using ExtractFirst = typename T::first_type;

constexpr bool CheckPrimaty(size_t value) {
  size_t limit = value / 2 + 1;
  for (size_t i = 2; i < limit; ++i) {
    if (value % i == 0) {
      return false;
    }
  }
  return true;
}
template <int value>
consteval int FindNextPrime() {
  for (size_t i = size_t(value); i <= value * 2; ++i) {
    if (CheckPrimaty(i)) {
      return int(i);
    }
  }
  // unreachable
  return value;
}

template <int From>
struct PrimeBase {
  constexpr static int Next = FindNextPrime<From>();
  using Head = ValueTag<Next>;
  using Tail = PrimeBase<Next + 1>;
};

using Fib =
    Map<ExtractFirst, Iterate<NextFib, std::pair<ValueTag<0>, ValueTag<1>>>>;
}  // namespace detail

// template <template <class> class F, TypeList TL>
// using Map = detail::Map<F, TL>;
using Nats = type_lists::Iterate<detail::Increment, value_types::ValueTag<0>>;
using detail::Fib;
using Primes = detail::PrimeBase<2>;
