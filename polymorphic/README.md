# Polymorphic Type Mapping in C++

A compile-time mapping mechanism for polymorphic types to fixed values, leveraging `dynamic_cast` and C++20's support for class-type non-type template parameters.

## Implemented Components

### `PolymorphicMapper` (in `string_mapper.hpp`)
- Maps polymorphic types (`Base&`) to `Target` values.
- Uses `Mapping<From, target>` entries, where `From` must inherit from `Base`.
- Returns `std::optional<Target>`; `std::nullopt` if no match.
- Compile-time validation ensures only valid mappings are used.
- Guaranteed `O(n)` `dynamic_cast` calls in runtime.

**Bonus:** Supports unordered mappings.

### `FixedString` (Compile-Time Strings)
- Stores a `const char*` at compile time with length constraints.
- Implicit conversion to `std::string_view`.
- Supports non-type template parameters.

### `"..."_cstr` Operator
- Converts string literals to `FixedString<256>`.
- Usable in template parameters and compile-time contexts.

## Example Usage
```cpp
class Animal { public: virtual ~Animal() = default; };
class Cat : public Animal {}; class Dog : public Animal {}; class Cow : public Animal {};

using MyMapper = PolymorphicMapper<
    Animal, FixedString<256>,
    Mapping<Cat, "Meow"_cstr>,
    Mapping<Dog, "Bark"_cstr>,
    Mapping<Cow, "Moo"_cstr>
>;

std::unique_ptr<Animal> some_animal{new Dog()};
std::string_view sound = *MyMapper::map(*some_animal);
ASSERT_EQ(sound, "Bark");
```
