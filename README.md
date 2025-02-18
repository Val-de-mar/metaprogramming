# Metaprogramming Course

A collection of advanced C++20 metaprogramming exercises, covering concepts such as type erasure, compile-time type sequences, polymorphic mapping, enumeration reflection, and smart wrappers.

Original repository: [Mrkol/metaprogramming-course](https://github.com/Mrkol/metaprogramming-course.git)

## Tasks Overview

### 1. **Span (`span/`)**
   - Custom implementation of `std::span` with minimal memory overhead.
   - Supports both static and dynamic extents.
   - Provides safe access with runtime bounds checking.

### 2. **Slice (`slice/`)**
   - Generalized `std::span` with an additional `stride` parameter.
   - Supports both compile-time and runtime extents and strides.
   - Provides slicing operations: `DropFirst`, `DropLast`, `Skip`, etc.

### 3. **Infinite Type Lists (`type_lists/`)**
   - Implements infinite type sequences with lazy instantiation.
   - Functional-style operations: `Map`, `Filter`, `Zip`, etc.
   - Includes compile-time number sequences (`Nats`, `Fib`, `Primes`).

### 4. **Polymorphic Mapping (`polymorphic/`)**
   - Maps polymorphic objects to compile-time string values.
   - Uses C++20 non-type template parameters for string storage.
   - Supports runtime mapping via `dynamic_cast`.

### 5. **Spy Wrapper (`spy/`)**
   - Logs accesses to wrapped objects via `operator->`.
   - Preserves standard concepts (`std::regular`, `std::movable`).
   - Supports move-only and copyable types.

### 6. **Enumerator Reflection (`enum/`)**
   - Retrieves enum size, values, and names at compile time.
   - Works for both `enum class` and unscoped enums.
   - Optimized for constant-time access.

## Requirements
- **C++20** or later.
- **GCC 10+**
