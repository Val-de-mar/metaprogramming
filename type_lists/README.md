# Type Sequences and Infinite Type Lists in C++

A metaprogramming exercise exploring lazy infinite type sequences, inspired by functional programming concepts.

## Implemented Concepts

### Type Sequences and Type Lists
- **`TypeSequence<TL>`** – Concept for infinite type sequences with `Head` and `Tail`.
- **`Empty<TL>`** – Concept for empty lists (derived from `Nil`).
- **`TypeList<TL>`** – Unified concept for finite and infinite lists.

### Type Tuple
- **`TTuple<Ts...>`** – Compact representation of finite type sequences.
- **`TypeTuple<TT>`** – Concept for type tuples.

## Implemented Operations (`type_lists` namespace)
- **Basic Operations**: `Cons<T, TL>`, `FromTuple<TT>`, `ToTuple<TL>`, `Repeat<T>`, `Replicate<N, T>`.
- **List Manipulation**: `Take<N, TL>`, `Drop<N, TL>`, `Iterate<F, T>`, `Cycle<TL>`.
- **Transformations**: `Map<F, TL>`, `Filter<P, TL>`.
- **Folds and Scans**: `Foldl<OP, T, TL>`, `Foldr<OP, T, TL>`, `Scanl<OP, T, TL>`.
- **Combinators**: `Zip2<L, R>`, `Zip<TL...>`, `Inits<TL>`, `Tails<TL>`.
- **Advanced**: `GroupBy<EQ, TL>` (bonus).

## Value Sequences (`fun_value_sequences.hpp`)
- **`ValueTag<V>`** – Encapsulates values in types.
- **Number Sequences**: `Nats` (natural numbers), `Fib` (Fibonacci), `Primes` (prime numbers).

