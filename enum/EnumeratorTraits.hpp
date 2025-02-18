#pragma once

#include <limits>
#include <string_view>
#include <type_traits>
#include <cstdint>
#include <array>
#include <iostream>
#include <ranges>
#include <algorithm>
#include <string_view>
#include <optional>
#include <numeric>
#include <utility>
#include <vector>

template <typename T>
concept EnumType = (std::is_enum_v<T>);

namespace rng = std::ranges; 
namespace vi = rng::views; 

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wenum-constexpr-conversion"
// your code for which the warning gets suppressed 
template<auto e>
constexpr std::string_view GetEnumName() {
    auto name = std::string_view(__PRETTY_FUNCTION__);
    auto rev = vi::reverse(name);
    auto a = std::min(rng::find(rev, ')'), rng::find(rev, ':'));
    auto end = std::min(a, rng::find(rev, ' '));
    if (*end == ')') {
        return "";
    }
    return std::string_view(end.base(), name.end() - 1);
}

template<auto e>
constexpr static auto enumName = GetEnumName<e>();



template<EnumType E, typename Positive>
struct HelperBig;

template<EnumType E, int... pos>
struct HelperBig<E, std::integer_sequence<int, pos...>> {
    constexpr static bool is_signed = std::is_signed_v<std::underlying_type_t<E>>;

    template<int val>
    static consteval auto Get() {
        return enumName<static_cast<E>(val)>;
    }

    template<int val>
    static consteval auto IsOk() {
        return !Get<val>().empty();
    }

    static consteval size_t SizePos() {
        return (0 + ... + size_t(IsOk<pos>()));
    }

    static consteval size_t SizeNeg() {
        if constexpr(is_signed) {
            return (0 + ... + size_t(IsOk<-pos - 1>()));
        } else {
            return 0;
        }
    }

    constexpr static auto pos_size = SizePos();
    constexpr static auto neg_size = SizeNeg();

    static consteval size_t Size() {
        return pos_size + neg_size;
    }


    template<int value>
    static consteval void helper(auto& neg_iter, auto& pos_iter) {
        if constexpr (IsOk<value>()) {
            *pos_iter = std::make_pair(Get<value>(), value);
            ++pos_iter;
        }
        if constexpr(is_signed && IsOk<- value - 1>()) {
            --neg_iter;
            *neg_iter = std::make_pair(Get<- value - 1>(), - value - 1);
        }
    }

    static consteval auto Good() {
        std::array<std::pair<std::string_view, int>, Size()> result;
        auto pos_iter = result.begin() + neg_size;
        auto neg_iter = result.begin() + neg_size;
        
        (... , helper<pos>(neg_iter, pos_iter));
        return result;
    }
    static constexpr auto at_array = Good();
};


template <class Enum, std::size_t MAXN = 512>
	requires std::is_enum_v<Enum>
struct EnumeratorTraits {
    using underlying = std::underlying_type_t<Enum>;

    consteval static int64_t CropPos() {
        auto limit = std::min(uint64_t(std::numeric_limits<underlying>::max()), uint64_t(std::numeric_limits<int32_t>::max() - 1));
        return std::min(int64_t(limit), int64_t(MAXN));
    }

    constexpr static int64_t MaxNum = CropPos();
    using dia = std::make_integer_sequence<int, MaxNum + 1>;
    using Base = HelperBig<Enum, dia>;
    static constexpr std::size_t size() noexcept {
        return Base::Size();
    }
    static constexpr Enum at(std::size_t i) noexcept {
        return static_cast<Enum>(Base::at_array[i].second);
    }
    static constexpr std::string_view nameAt(std::size_t i) noexcept {
        return Base::at_array[i].first;
    }
};

#pragma clang diagnostic pop