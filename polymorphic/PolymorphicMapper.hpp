#pragma once

#include <concepts>
#include <optional>
#include <FixedString.hpp>

template<typename... T>
struct TTuple {};

template<class TT>
concept TypeTuple = requires(TT t) { []<class... Ts>(TTuple<Ts...>&){}(t); };

template <class From, auto target>
struct Mapping {
  using Key = From;
  using TTarget = decltype(target);

  template<typename Target, typename Base>
  static std::optional<Target> TryExtract(const Base& object) {
    if (dynamic_cast<const From*>(&object)) {
      return {target};
    }
    return std::nullopt;
  }
};

template <template <class> class Proj, TypeTuple TT>
struct AllSameBase {
  constexpr static bool Value = true;
};

template <template <class> class Proj, typename T, typename... Tail>
struct AllSameBase<Proj, TTuple<T, Tail...>> {
  constexpr static bool Value = (true && ... && std::same_as<Proj<T>, Proj<Tail>>);
};

// template <template <class> class Proj, typename... T>
// concept AllSame = (AllSameBase<Proj<T>...>::Value);

template <typename TT, template <class> class Proj>
concept AllSame = TypeTuple<TT> && (AllSameBase<Proj, TT>::Value);

template <typename T>
using GetTTarget = typename T::TTarget;

template <class Base, class Target, AllSame<GetTTarget> Mappings>
struct PolymorphicMapperOrdered;

template <class Base, class Target, class... Mappings>
requires AllSame<TTuple<Mappings...>, GetTTarget>
struct PolymorphicMapperOrdered<Base, Target, TTuple<Mappings...>> : Mappings...{
  static std::optional<Target> map(const Base& object) {
    std::optional<Target> result;
    auto op = [](std::optional<Target>(*v)(const Base&), std::optional<Target>& result, const Base& object) {
      result = v(object);
      return bool(result);
    };
    [[maybe_unused]] bool found = (false || ... || op(&Mappings::template TryExtract<Target, Base>, result, object));
    return result;
  }
};

namespace detail {

template <TypeTuple T1, TypeTuple T2>
struct Merge2Base;

template <typename... T1, typename... T2>
struct Merge2Base<TTuple<T1...>, TTuple<T2...>> {
  using Type = TTuple<T1..., T2...>;
};

template <TypeTuple T1, TypeTuple T2>
using Merge2 = typename Merge2Base<T1, T2>::Type;

template <TypeTuple ...T>
struct MergeBase {
  using Type = TTuple<>;
};

template <TypeTuple Head, TypeTuple... Tail>
struct MergeBase<Head, Tail...> {
  using Type = Merge2<Head, typename MergeBase<Tail...>::Type>;
  static_assert(TypeTuple<Type>);
};

template <TypeTuple... T>
using Merge = typename MergeBase<T...>::Type;

}  // namespace detail

using detail::Merge;

template<TypeTuple T>
struct UniqueBase;

template<typename Head, typename... Tail>
struct UniqueBase<TTuple<Head, Tail...>> {
  static constexpr bool FirstIsUnique = (true && ... && !std::same_as<Head, Tail>);
  static constexpr bool Value = FirstIsUnique && UniqueBase<TTuple<Tail...>>::Value;
};

template<>
struct UniqueBase<TTuple<>> {
  static constexpr bool Value = true;
};

template<typename T>
concept UniqueTuple = TypeTuple<T> && (UniqueBase<T>::Value);

template<typename... T>
concept UniqueSeq = UniqueTuple<TTuple<T...>>;

template <template <class> class Proj,typename... List>
struct FilterDerivators;

template <template <class> class Proj, typename T, typename Head, typename... Tail>
requires (!std::derived_from<Proj<Head>, Proj<T>>)
struct FilterDerivators<Proj, T, Head, Tail...> {
  using Derivators = typename FilterDerivators<Proj, T, Tail...>::Derivators;
  using Remains = detail::Merge2<TTuple<Head>, typename FilterDerivators<Proj, T, Tail...>::Remains>;
};

template <template <class> class Proj, typename T, typename Head,typename... Tail>
requires std::derived_from<Proj<Head>, Proj<T>> 
struct FilterDerivators<Proj, T, Head, Tail...> {
  using Derivators = Merge<TTuple<Head>, typename FilterDerivators<Proj, T, Tail...>::Derivators>;
  using Remains = FilterDerivators<Proj, T, Tail...>::Remains;
};

template <template <class> class Proj, typename T>
struct FilterDerivators<Proj, T> {
  using Derivators = TTuple<>;
  using Remains = TTuple<>;
};

template <template <class> class Proj, UniqueTuple Tuple>
struct TopologicallySortedBase;

template<template <class> class Proj, typename T, typename... List>
requires UniqueTuple<TTuple<T, List...>>
struct TopologicallySortedBase<Proj, TTuple<T, List...>> {
  using Splitted = FilterDerivators<Proj, T, List...>;
  using Reordered = Merge<typename Splitted::Derivators, TTuple<T>, typename Splitted::Remains>;
  static_assert(TypeTuple<Merge<typename Splitted::Derivators, TTuple<T>, typename Splitted::Remains>>);
  using Type = TopologicallySortedBase<Proj, Reordered>::Type;
};

template<template <class> class Proj, typename T, typename... List>
requires UniqueTuple<TTuple<T, List...>> && std::same_as<typename FilterDerivators<Proj, T, List...>::Derivators, TTuple<>>
struct TopologicallySortedBase<Proj, TTuple<T, List...>> {
  using Type = Merge<TTuple<T>, typename TopologicallySortedBase<Proj, TTuple<List...>>::Type>;
};

template<template <class> class Proj>
struct TopologicallySortedBase<Proj, TTuple<>> {
  using Type = TTuple<>;
};

template<typename T>
using Id = T;

template<UniqueTuple Tuple, template <class> class Proj = Id>
using TopologicallySorted = typename TopologicallySortedBase<Proj, Tuple>::Type;

template<typename T>
using GetKey = typename T::Key;

template <class Base, class Target, class... Mappings>
using PolymorphicMapper = PolymorphicMapperOrdered<Base, Target, TopologicallySorted<TTuple<Mappings...>, GetKey>>;
