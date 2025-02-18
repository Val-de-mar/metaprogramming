#pragma once

#include <utility>
namespace type_tuples {

template <class... Ts>
struct TTuple {};

template <class TT>
concept TypeTuple = requires(TT t) { []<class... Ts>(TTuple<Ts...>&) {}(t); };

namespace detail {

template <class T>
struct Push;

template <typename... T>
struct Push<TTuple<T...>> {
  template <typename... Head>
  using Front = TTuple<Head..., T...>;

  template <typename... Tail>
  using Back = TTuple<T..., Tail...>;
};

template <TypeTuple T1, TypeTuple T2>
struct MergeBase;

template <typename... T1, typename... T2>
struct MergeBase<TTuple<T1...>, TTuple<T2...>> {
  using Type = TTuple<T1..., T2...>;
};

template <TypeTuple T1, TypeTuple T2>
using Merge = typename MergeBase<T1, T2>::Type;

}  // namespace detail

using detail::Merge;

}  // namespace type_tuples
