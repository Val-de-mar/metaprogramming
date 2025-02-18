#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>
#include <type_tuples.hpp>
#include <value_types.hpp>

namespace type_lists {

template <class TL>
concept TypeSequence = requires {
  typename TL::Head;
  typename TL::Tail;
};

struct Nil {};

template <class TL>
concept Empty = std::derived_from<TL, Nil>;

template <class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;

template <typename T, TypeList TL>
struct Cons {
  using Head = T;
  using Tail = TL;
};

template <typename T>
struct Repeat {
  using Head = T;
  using Tail = Repeat<T>;
};

namespace detail {
using namespace type_tuples;
template <TypeTuple TS>
struct FromTupleBase;

template <typename Head, typename... Tail>
struct FromTupleBase<TTuple<Head, Tail...>> {
  using Type = Cons<Head, typename FromTupleBase<TTuple<Tail...>>::Type>;
};

template <>
struct FromTupleBase<TTuple<>> {
  using Type = Nil;
};

template <TypeTuple TT>
using FromTuple = typename FromTupleBase<TT>::Type;

template <TypeList T>
struct ToTupleBase;

template <TypeSequence TL>
struct ToTupleBase<TL> {
  using Type = Merge<TTuple<typename TL::Head>,
                     typename ToTupleBase<typename TL::Tail>::Type>;
};

template <Empty T>
struct ToTupleBase<T> {
  using Type = TTuple<>;
};

template <TypeList TL>
using ToTuple = typename ToTupleBase<TL>::Type;

template <TypeList TL, size_t N>
struct SizeGreaterBase;

template <TypeSequence TL, size_t N>
struct SizeGreaterBase<TL, N> {
  constexpr static bool Value =
      SizeGreaterBase<typename TL::Tail, N - 1>::Value;
};

template <TypeSequence TL>
struct SizeGreaterBase<TL, 0> {
  constexpr static bool Value = true;
};

template <Empty E, size_t N>
struct SizeGreaterBase<E, N> {
  constexpr static bool Value = N == 0;
};

template <typename TL, size_t N>
concept AtLeast = SizeGreaterBase<TL, N>::Value;

template <size_t N, TypeList TL>
struct CutBase {
  using Prefix = Cons<typename TL::Head,
                      typename CutBase<N - 1, typename TL::Tail>::Prefix>;
  using Suffix = typename CutBase<N - 1, typename TL::Tail>::Suffix;
};

template <size_t N, Empty TL>
  requires(N != 0)
struct CutBase<N, TL> {
  using Prefix = Nil;
  using Suffix = Nil;
};

template <TypeList TL>
struct CutBase<0, TL> {
  using Prefix = Nil;
  using Suffix = TL;
};

template <template <class> class F, TypeList TL>
struct Map {
  using Head = F<typename TL::Head>;
  using Tail = Map<F, typename TL::Tail>;
};

template <template <class> class F, Empty TL>
struct Map<F, TL> : Nil {};

template <template <class> class FFilter, TypeList TL>
struct Filter;

template <template <class> class FFilter, TypeSequence TL>
  requires(FFilter<typename TL::Head>::Value)
struct Filter<FFilter, TL> {
  using Head = typename TL::Head;
  using Tail = Filter<FFilter, typename TL::Tail>;
};

template <template <class> class FFilter, TypeSequence TL>
  requires(!FFilter<typename TL::Head>::Value)
struct Filter<FFilter, TL> : Filter<FFilter, typename TL::Tail> {};

template <template <class> class FFilter, Empty TL>
struct Filter<FFilter, TL> : Nil {};

template <TypeList Full, TypeList Current = Full>
struct Cycle {
  using Actual = std::conditional_t<Empty<Current>, Full, Current>;
  using Head = typename Actual::Head;
  using Tail = Cycle<Full, typename Actual::Tail>;
};

template <Empty Full, TypeList Current>
struct Cycle<Full, Current> : Nil {};

template <TypeList TL>
struct Cut {
  template <class Value>
    requires AtLeast<TL, Value::Value>
  using Prefix = typename CutBase<Value::Value, TL>::Prefix;

  template <class Value>
    requires AtLeast<TL, Value::Value>
  using Suffix = typename CutBase<Value::Value, TL>::Suffix;
};

template <TypeList TL, TypeList Prefix = Nil>
struct Inits {
  using AppendPrefix = Merge<ToTuple<Prefix>, TTuple<typename TL::Head>>;
  using Head = Prefix;
  using Tail = Inits<typename TL::Tail, FromTuple<AppendPrefix>>;
};
template <Empty TL, TypeList Prefix>
struct Inits<TL, Prefix> {
  using Head = Prefix;
  using Tail = Nil;
};

template <TypeList TL>
struct Tails {
  using Head = TL;
  using Tail = Tails<typename TL::Tail>;
};
template <Empty TL>
struct Tails<TL> {
  using Head = Nil;
  using Tail = Nil;
};

template <template <class, class> class F, class T, TypeList TL>
struct Scanl {
  using Head = T;
  using Tail = Scanl<F, F<T, typename TL::Head>, typename TL::Tail>;
};

template <template <class, class> class F, class T, Empty TL>
struct Scanl<F, T, TL> {
  using Head = T;
  using Tail = Nil;
};

template <template <class, class> class F, class T, TypeList TL>
struct FoldlBase {
  using Partial = F<T, typename TL::Head>;
  using Type = typename FoldlBase<F, Partial, typename TL::Tail>::Type;
};

template <template <class, class> class F, class T, Empty TL>
struct FoldlBase<F, T, TL> : Nil {
  using Type = T;
};

template <template <class, class> class F, class T, TypeList TL>
using Foldl = typename FoldlBase<F, T, TL>::Type;

template <typename T>
using ExtractHead = typename T::Head;

template <typename T>
using ExtractTail = typename T::Tail;

template <typename... T>
concept AnyEmpty = (Empty<T> || ...);

template <typename... T>
concept EveryList = (TypeList<T> && ...);

template <typename... TL>
  requires EveryList<TL...>
struct Zip {
  using Head = TTuple<ExtractHead<TL>...>;
  using Tail = Zip<ExtractTail<TL>...>;
};

template <TypeList... TL>
  requires EveryList<TL...> && AnyEmpty<TL...>
struct Zip<TL...> : Nil {};

template <template <class> class Predicate, TypeList TL, TypeTuple CurrentPrefix>
struct WhileTrue;

template <template <class> class Predicate, TypeSequence TL,
          TypeTuple CurrentPrefix>
  requires(Predicate<typename TL::Head>::Value)
struct WhileTrue<Predicate, TL, CurrentPrefix>
    : WhileTrue<Predicate, typename TL::Tail,
                Merge<CurrentPrefix, TTuple<typename TL::Head>>> {};

template <template <class> class Predicate, TypeSequence TL,
          TypeTuple CurrentPrefix>
  requires(!Predicate<typename TL::Head>::Value)
struct WhileTrue<Predicate, TL, CurrentPrefix> {
  using Prefix = CurrentPrefix;
  using Remains = TL;
};

template <template <class> class Predicate, Empty TL, TypeTuple CurrentPrefix>
struct WhileTrue<Predicate, TL, CurrentPrefix> {
  using Prefix = CurrentPrefix;
  using Remains = TL;
};

template <template <class, class> class EQ, TypeList TL>
struct GroupBy;

template <template <class, class> class EQ, TypeSequence TL>
struct GroupBy<EQ, TL> {
 private:
  template <typename U>
  struct Predicate {
    static constexpr bool Value = EQ<typename TL::Head, U>::Value;
  };

  using FiltratedPrefix = WhileTrue<Predicate, typename TL::Tail, TTuple<typename TL::Head>>;

 public:
  using Head = FromTuple<typename FiltratedPrefix::Prefix>;
  using Tail = GroupBy<EQ, typename FiltratedPrefix::Remains>;
};

template <template <class, class> class EQ, Empty TL>
struct GroupBy<EQ, TL> : Nil {};

}  // namespace detail

using detail::FromTuple;
using detail::ToTuple;

template <size_t N, detail::AtLeast<N> TL>
using TakeChecked = typename detail::CutBase<N, TL>::Prefix;

template <size_t N, detail::AtLeast<N> TL>
using DropChecked = typename detail::CutBase<N, TL>::Suffix;

template <size_t N, TypeList TL>
using Take = typename detail::CutBase<N, TL>::Prefix;

template <size_t N, TypeList TL>
using Drop = typename detail::CutBase<N, TL>::Suffix;

using detail::Map;
using detail::Filter;

template <template <class> class F, typename T>
struct Iterate {
  using Head = T;
  using Tail = Iterate<F, F<T>>;
};

template <TypeList TL>
using Cycle = detail::Cycle<TL>;

using detail::Foldl;
using detail::GroupBy;
using detail::Inits;
using detail::Scanl;
using detail::Tails;
using detail::Zip;

template <TypeList TL1, TypeList TL2>
using Zip2 = Zip<TL1, TL2>;

template <size_t N, class T>
struct Replicate {
  using Head = T;
  using Tail = Replicate<N - 1, T>;
};

template <class T>
struct Replicate<0, T> : Nil {};

}  // namespace type_lists

template <typename T>
constexpr auto See();
