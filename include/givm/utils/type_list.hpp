#ifndef GIVM_UTILS_TYPE_LIST_HPP
#define GIVM_UTILS_TYPE_LIST_HPP

#include <cstddef>
#include <type_traits>
#include <utility>

namespace givm
{
    template<class T, size_t I>
    struct type_list_unit
    {
        static consteval size_t index_of(std::type_identity<T>)
        {
            return I;
        };

        static consteval std::type_identity<T> get(std::integral_constant<size_t, I>)
        {
            return {};
        };
    };

    template<class TSeq, class...T>
    struct type_list_impl;

    template<>
    struct type_list_impl<std::index_sequence<>>
    {
        static consteval size_t size() noexcept
        {
            return 0;
        }

        template<template<class...> class U>
        using apply = U<>;

        template<class F>
        static constexpr bool all(F&& fn)
        {
            return true;
        }

        template<class F>
        static constexpr void each(F&& fn)
        {}
    };

    template<size_t...I, class...T>
    struct type_list_impl<std::index_sequence<I...>, T...> : type_list_unit<T, I>...
    {
        using type_list_unit<T, I>::index_of...;
        using type_list_unit<T, I>::get...;

        static consteval size_t size() noexcept
        {
            return sizeof...(T);
        }

        template<size_t J>
        using type_at = decltype(get(std::integral_constant<size_t, J>{}))::type;

        template<template<class...> class U>
        using apply = U<T...>;

        template<class U> requires requires{ index_of(std::type_identity<U>{}); }
        static consteval size_t index_of() noexcept
        {
            return index_of(std::type_identity<U>{});
        }

        template<class F>
        static constexpr bool all(F&& fn)
        {
            return (... && fn.template operator()<T>());
        }

        template<class F>
        static constexpr void each(F&& fn)
        {
            (..., fn.template operator()<T>());
        }
    };

    template<class...T>
    struct type_list : type_list_impl<std::index_sequence_for<T...>, T...>{};

    template<class... TList>
    using type_list_cat = decltype([]{
        if constexpr (sizeof...(TList) == 0)
        {
            return type_list<>{};
        }
        else if constexpr (sizeof...(TList) == 1)
        {
            return (..., TList{});
        }
        else return []<class... TLeft, class... TRight, class... TRest>
        (this auto&& self, type_list<TLeft...>, type_list<TRight...>, TRest...)
        {
            const auto head = type_list<TLeft..., TRight...>{};
            if constexpr (sizeof...(TRest) == 0)
            {
                return head;
            }
            else
            {
                return self(head, TRest{}...);
            }
        }(TList{}...);
    }());

}

#endif
