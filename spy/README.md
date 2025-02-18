# Spy: Smart Non-Pointer Wrapper in C++

A C++20 wrapper for logging accesses to an object while preserving fundamental concepts.

## Features
- **Value-based storage**: Stores the wrapped object by value, not by pointer.
- **Logging support**: Calls a user-defined logger after each `operator->` access.
- **Concept preservation**: Ensures `Spy<T>` satisfies `std::movable`, `std::copyable`, `std::semiregular`, and `std::regular` if `T` does.
- **Transparent access**: `s->member` acts as `s.get().member`.
- **Copy and move semantics**:
  - Copying creates a separate object with independent access counting.
  - Moving transfers the wrapped object and logger.

## Implementation Constraints
- **No use of `std::function` or `std::any`**.
- **Logger support**:
  - Can be move-only if `T` is move-only.
  - Must not change during access in a single expression.

## Example Usage
```cpp
struct Holder {
    int x = 0;
    bool isPositive() const { return x > 0; }
};

Spy s{Holder{}};
s.setLogger([](auto n) { std::cout << n << std::endl; });

s->isPositive() && s->x--; // prints 1
s->x++ + s->x++; // prints 2
s->isPositive() && s->x--; // prints 2
```