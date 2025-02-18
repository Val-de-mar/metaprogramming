#include <bits/iterator_concepts.h>

#include <array>
#include <cassert>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <span>
#include <type_traits>
#include <algorithm>

inline constexpr std::ptrdiff_t dynamic_stride = -1;
namespace {
namespace Util {
template <class T, T Value, T ReservedValue>
class MaybeCompileTimeStorage {
 public:
  MaybeCompileTimeStorage() = default;

 protected:
  constexpr T Get() const { return Value; }

  [[gnu::always_inline]] void Set(T value) { assert(Value == value); }
};

template <class T, T Value>
class MaybeCompileTimeStorage<T, Value, Value> {
 public:
  MaybeCompileTimeStorage() = default;

 protected:
  T value_;
  [[gnu::always_inline]] T Get() const { return value_; }

  [[gnu::always_inline]] void Set(T value) { value_ = value; }
};

template <ptrdiff_t Value>
struct TStride
    : public MaybeCompileTimeStorage<ptrdiff_t, Value, dynamic_stride> {};

template <std::size_t Value>
struct TExtent
    : public MaybeCompileTimeStorage<std::size_t, Value, std::dynamic_extent> {
};

template <class T, std::ptrdiff_t Stride>
struct SliceIteratorProperties {};

template <class T>
struct SliceIteratorProperties<T, 1> {
  using iterator_category = std::contiguous_iterator_tag;
};

}  // namespace Util
}  // namespace

template <class T, std::ptrdiff_t stride>
class SliceIterator : Util::TStride<stride>,
                      public Util::SliceIteratorProperties<T, stride> {
  using TStride = Util::TStride<stride>;
  using TThis = SliceIterator<T, stride>;

 public:
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cv_t<T>;
  using element_type = int;
  using pointer = T*;
  using reference = T&;

 public:
  SliceIterator() = default;
  SliceIterator(const SliceIterator<T, stride>& other) = default;

  SliceIterator(pointer data, std::ptrdiff_t skip) : data_(data) {
    TStride::Set(skip);
  }

  [[gnu::always_inline]] T& operator*() const { return *data_; }
  [[gnu::always_inline]] T* operator->() const { return data_; }

  [[gnu::always_inline]] T& operator[](ptrdiff_t place) const {
    return *(data_ + (TStride::Get() * place));
  }

  TThis& operator++() {
    data_ += TStride::Get();
    return *this;
  }
  TThis& operator--() {
    data_ -= TStride::Get();
    return *this;
  }
  TThis operator++(int) {
    auto copy = *this;
    data_ += TStride::Get();
    return copy;
  }
  TThis operator--(int) {
    auto copy = *this;
    data_ -= TStride::Get();
    return copy;
  }

  TThis& operator+=(ptrdiff_t delta) {
    data_ += TStride::Get() * delta;
    return *this;
  }
  TThis& operator-=(ptrdiff_t delta) {
    data_ -= TStride::Get() * delta;
    return *this;
  }
  TThis operator+(ptrdiff_t delta) const {
    auto copy = *this;
    copy += delta;
    return copy;
  }
  TThis operator-(ptrdiff_t delta) const {
    auto copy = *this;
    copy -= delta;
    return copy;
  }

  ptrdiff_t operator-(const TThis& other) const {
    return (data_ - other.data_) / TStride::Get();
  }

  auto operator<=>(const TThis& other) const { return data_ <=> other.data_; }

  bool operator==(const TThis& other) const { return data_ == other.data_; }

 private:
  T* data_;
};

template <class T, std::ptrdiff_t Stride>
SliceIterator<T, Stride> operator+(ptrdiff_t lhs,
                                   const SliceIterator<T, Stride>& rhs) {
  auto copy = rhs;
  copy += lhs;
  return copy;
}

template <class T, std::size_t extent = std::dynamic_extent,
          std::ptrdiff_t stride = 1>
class Slice : protected Util::TStride<stride>, protected Util::TExtent<extent> {
  using TStride = Util::TStride<stride>;
  using TExtent = Util::TExtent<extent>;
  using TThis = Slice<T, extent, stride>;

  template <size_t count>
  const static size_t Subtract =
      extent == std::dynamic_extent ? extent : extent - count;

 public:
  using iterator = SliceIterator<T, stride>;
  using reverse_iterator = std::reverse_iterator<iterator>;

  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;

 public:
  template <class U>
  Slice(U& container) : Slice(container.begin(), container.end(), 1) {}

  template <std::contiguous_iterator It>
  Slice(It first, It last, std::ptrdiff_t skip)
      : Slice<T, extent>(first, size_t(last - first), skip) {}

  template <std::contiguous_iterator It>
  Slice(It first, std::size_t count, std::ptrdiff_t skip)
      : data_(std::to_address(first)) {
    TExtent::Set(count);
    TStride::Set(skip);
  }

  Slice()
    requires(extent == std::dynamic_extent)
      : data_(nullptr) {
    TExtent::Set(0);
  }

  template <typename T1, size_t extent1, std::ptrdiff_t stride1>
    requires(std::same_as<T1, T> || std::same_as<T1, std::add_const_t<T>>) &&
            (extent1 == extent || extent1 == std::dynamic_extent) &&
            (stride1 == stride || stride1 == dynamic_stride)
  bool operator==(const Slice<T1, extent1, stride1>& rhs) const {
    return std::ranges::equal(*this, rhs);
  }

  operator Slice<std::add_const_t<T>, extent, stride>()
    requires(!std::is_const_v<T>)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  operator Slice<std::add_const_t<T>, std::dynamic_extent, stride>()
    requires(!std::is_const_v<T> && extent != std::dynamic_extent)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  operator Slice<std::add_const_t<T>, extent, dynamic_stride>()
    requires(!std::is_const_v<T> && stride != dynamic_stride)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  operator Slice<std::add_const_t<T>, std::dynamic_extent, dynamic_stride>()
    requires(!std::is_const_v<T> && stride != dynamic_stride &&
             extent != std::dynamic_extent)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  operator Slice<T, std::dynamic_extent, stride>()
    requires(extent != std::dynamic_extent)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  operator Slice<T, extent, dynamic_stride>()
    requires(stride != dynamic_stride)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  operator Slice<T, std::dynamic_extent, dynamic_stride>()
    requires(stride != dynamic_stride && extent != std::dynamic_extent)
  {
    return {data_, TExtent::Get(), TStride::Get()};
  }

  iterator begin() const { return {data_, TStride::Get()}; }

  iterator end() const {
    return {data_ + TExtent::Get() * TStride::Get(),
            TStride::Get()};
  }

  [[gnu::always_inline]] T& operator[](ptrdiff_t place) const {
    return *(data_ + (TStride::Get() * place));
  }

  auto rbegin() const { return std::reverse_iterator(end()); }
  auto rend() const { return std::reverse_iterator(begin()); }

  constexpr size_t Size() const { return TExtent::Get(); }

  const_pointer Data() const { return data_; }

  constexpr size_t Stride() const { return TStride::Get(); }

  Slice<T, std::dynamic_extent, stride> First(std::size_t count) const {
    return {data_, count, TStride::Get()};
  }

  template <std::size_t count>
    requires(count <= extent)
  Slice<T, count, stride> First() const {
    return {
        data_,
        count /*std::min(count * TStride::Get(), TExtent::Get())*/,
        TStride::Get()};
  }

  Slice<T, std::dynamic_extent, stride> Last(std::size_t count) const {
    assert((Size() - count) >= 0);
    auto delta = (Size() - count) * TStride::Get();
    return {data_ + delta, count /* TExtent::Get() - delta */,
            TStride::Get()};
  }

  template <std::size_t count>
    requires(count <= extent)
  Slice<T, count, stride> Last() const {
    auto delta = (Size() - count) * TStride::Get();
    return {data_ + delta, count /* TExtent::Get() - delta */,
            TStride::Get()};
  }

  Slice<T, std::dynamic_extent, stride> DropFirst(std::size_t count) const {
    assert((Size() - count) >= 0);
    auto delta = count * TStride::Get();
    return {data_ + delta, TExtent::Get() - count, TStride::Get()};
  }

  template <std::size_t count>
    requires(extent >= count)
  Slice<T, Subtract<count>, stride> DropFirst() const {
    return {data_ + count * TStride::Get(), TExtent::Get() - count,
            TStride::Get()};
  }

  Slice<T, std::dynamic_extent, stride> DropLast(std::size_t count) const {
    assert((Size() - count) >= 0);
    return {data_, TExtent::Get() - count, TStride::Get()};
  }

  template <std::size_t count>
    requires(extent >= count)
  Slice<T, Subtract<count>, stride> DropLast() const {
    return {data_, TExtent::Get() - count, TStride::Get()};
  }

  Slice<T, std::dynamic_extent, dynamic_stride> Skip(
      std::ptrdiff_t skip) const {
    return Slice<T, std::dynamic_extent, dynamic_stride>{
        data_, (TExtent::Get() + skip - 1) / skip,
        TStride::Get() * skip};
  }

  template <std::ptrdiff_t skip>
    requires(skip > 0)
  Slice<T, extent == std::dynamic_extent ? extent : (extent + skip - 1) / skip,
        stride == dynamic_stride ? stride : stride * skip>
  Skip() const {
    return {data_, (TExtent::Get() + skip - 1) / skip,
            TStride::Get() * skip};
  }

 private:
  T* data_;
};

template <typename T, size_t Count>
Slice(std::array<T, Count>&) -> Slice<T, Count>;

template <class Container>
Slice(Container&) -> Slice<typename Container::value_type, std::dynamic_extent>;

template <typename It, typename End>
Slice(It, End) -> Slice<typename It::value_type, std::dynamic_extent>;

template <typename It, typename End, typename Integer>
Slice(It, End, Integer)
    -> Slice<typename It::value_type, std::dynamic_extent, dynamic_stride>;
