#include <bits/iterator_concepts.h>
#include <concepts>
#include <cassert>
#include <cstdlib>
#include <iterator>
#include <memory>
#include <span>

namespace {
template <std::size_t Extent = std::dynamic_extent>
class SpanStorage {
 public:
  SpanStorage() {}
 protected:

  constexpr size_t GetExtent() const { return Extent; }

  [[gnu::always_inline]] void SetExtent(size_t extent) {
    assert(Extent == extent);
  }
};

template <>
class SpanStorage<std::dynamic_extent> {
 public:
  SpanStorage() {}
 protected:
  size_t extent_;
  [[gnu::always_inline]] size_t GetExtent() const { return extent_; }

  [[gnu::always_inline]] void SetExtent(size_t extent) {
    extent_ = extent;
  }
};
}  // namespace

template <class T, std::size_t Extent = std::dynamic_extent>
class Span : SpanStorage<Extent> {
 public:
  
  template <std::contiguous_iterator It>
  explicit(Extent != std::dynamic_extent) constexpr Span(It first, size_t count)
      : SpanStorage<Extent>()
      , data_(std::to_address(first)) {
    this->SetExtent(count);
  }

  template <std::contiguous_iterator It, std::contiguous_iterator End>
  explicit(Extent != std::dynamic_extent) constexpr Span(It begin, End end)
      : Span<T, Extent>(begin, size_t(end - begin)) {}

  template<size_t Count>
  constexpr Span(std::array<T, Count>& array)
      : Span<T, Extent>(array.begin(), array.end()) {
        static_assert(Count == Extent);
      }

  template<class Container>
  explicit(Extent != std::dynamic_extent)
  constexpr Span(Container& container)
      : Span<T, Extent>(container.begin(), container.end()) {}

  T* begin() const { return data_; }
  T* end() const { return data_ + this->GetExtent(); }

  auto rbegin() const { return std::reverse_iterator(end()); }
  auto rend() const { return std::reverse_iterator(begin()); }

  T& operator[](size_t place) const { return data_[place]; }

  T& Front() const { return data_[0]; };
  T& Back() const { return data_[this->GetExtent() - 1]; };

  template< std::size_t Count >
  Span<T, Count> First() const {
    static_assert(Count <= Extent);
    return Span<T, Count>(data_, Count);
  }

  Span<T, std::dynamic_extent> First(size_t count) const {
    return Span<T, std::dynamic_extent>(data_, count);
  }

  template< std::size_t Count >
  Span<T, Count> Last() const {
    static_assert(Count <= Extent);
    return Span<T, Count>(end() - Count, Count);
  }

  Span<T, std::dynamic_extent> Last(size_t count) const {
    return Span<T, std::dynamic_extent>(end() - count, count);
  }

  constexpr size_t Size() const {
    return this->GetExtent();
  }
  constexpr auto Data() const {
    return data_;
  }

  using iterator = T*;
  using reverse_iterator = std::reverse_iterator<iterator>;

  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
 private:
  T* data_;
};

template<typename T, size_t Count>
Span(std::array<T, Count>&) -> Span<T, Count>;

template<class Container>
Span(Container&) -> Span<typename Container::value_type, std::dynamic_extent>;

template<typename It, typename End>
Span(It, End) -> Span<typename It::value_type, std::dynamic_extent>;

template<typename It, typename IntergerType>
requires std::constructible_from<std::size_t, IntergerType>
Span(It, IntergerType) -> Span<typename It::value_type, std::dynamic_extent>;
