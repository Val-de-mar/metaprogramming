# EnumeratorTraits: Compile-Time Enumeration Reflection in C++

A C++20 utility for retrieving enumeration metadata at compile time, including enumerator count, values, and names.

## Features
- **Works with both scoped (`enum class`) and unscoped enums**.
- **Compile-time retrieval**:
  - `size()` – number of enumerators.
  - `at(i)` – enumerator at index `i`, ordered by value.
  - `nameAt(i)` – name of the enumerator at index `i`.
- **Guaranteed uniqueness**: No duplicate values in the enumeration.
- **Optimized for performance**:
  - **Constant-time (`O(1)`) access** for `size()`, `at(i)`, and `nameAt(i)`.
  - **Linear (`O(N)`) memory** usage relative to `MAXN`.

## Example Usage
```cpp
#include "enumerators/EnumeratorTraits.hpp"

enum Fruit : unsigned short { APPLE, MELON };
enum class Shape { SQUARE, CIRCLE = 5, LINE, POINT = -2 };

static_assert(EnumeratorTraits<Shape>::size() == 4);
static_assert(EnumeratorTraits<Fruit>::size() == 2);

static_assert(EnumeratorTraits<Fruit>::nameAt(0) == "APPLE");
static_assert(EnumeratorTraits<Fruit>::at(1) == Fruit::MELON);

static_assert(EnumeratorTraits<Shape>::nameAt(0) == "POINT");
static_assert(EnumeratorTraits<Shape>::at(3) == Shape::LINE);
```