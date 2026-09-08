#ifndef GIVM_UTILS_STACK_HPP
#define GIVM_UTILS_STACK_HPP

#include <array>
#include <algorithm>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstring>
#include <memory>
#include <new>
#include <ranges>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include "debug.hpp"
#include "maybe_const.hpp"
#include "type_list.hpp"

#include "../macro_define.hpp"

namespace givm
{
    inline constexpr size_t max_alignment = alignof(std::max_align_t);

    using stack_count_t = size_t;

    namespace detail
    {
        struct dynamic_array_span
        {
            size_t offset_to_frame_end;
            stack_count_t count;
        };

        constexpr size_t align(size_t base, size_t alignment) noexcept
        {
            return (base + alignment - 1) & ~(alignment - 1);
        }

        struct layout_info
        {
            size_t size;
            size_t alignment;
        };

        template<class T>
        inline constexpr layout_info layout_info_of = []
        {
            if constexpr (std::is_unbounded_array_v<T>)
            {
                static_assert(std::is_trivially_copyable_v<std::remove_extent_t<T>>);
                return layout_info{ sizeof(dynamic_array_span), alignof(dynamic_array_span) };
            }
            else
            {
                static_assert(std::is_trivially_copyable_v<T>);
                return layout_info{ sizeof(T), alignof(T) };
            }
        }();

        template<size_t N>
        constexpr auto offsets_to_end(const std::array<layout_info, N>& infos)
        {
            std::array<size_t, N> result;
            result[N - 1] = infos[N - 1].size;
            for(size_t i = N - 2; i < N; --i)
            {
                result[i] = align(result[i + 1], infos[i].alignment) + infos[i].size;
            }
            return result;
        }

        template<size_t N>
        constexpr size_t frame_size(const std::array<layout_info, N>& infos)
        {
            size_t result = infos[N - 1].size;
            for(size_t i = N - 2; i < N; --i)
            {
                result = align(result, infos[i].alignment) + infos[i].size;
            }
            return result;
        }

        template<std::array ElementLayouts, std::same_as<stack_count_t>...TCount>
        constexpr auto caculate_dynamic_array_spans(size_t fixed_part, TCount...counts) noexcept
        {
            constexpr size_t n = sizeof...(TCount);
            std::array<dynamic_array_span, n> result;
            std::array<stack_count_t, n> count_values{ counts... };

            size_t dynamic_part = 0;
            for(size_t i = 0; i < n; ++i)
            {
                dynamic_part = align(dynamic_part, ElementLayouts[i].alignment);
                result[i] = {
                    .offset_to_frame_end = dynamic_part,
                    .count = count_values[i]
                };
                dynamic_part += ElementLayouts[i].size * count_values[i];
            }

            const size_t aligned_frame_size = align(dynamic_part + fixed_part, max_alignment);
            for(size_t i = 0; i < n; ++i)
            {
                result[i].offset_to_frame_end = aligned_frame_size - result[i].offset_to_frame_end;
            }

            return result;
        }

        template<class... T>
        concept dynamic_array_prefix = []
        {
            bool fixed_seen = false;
            for(const bool is_dynamic : std::array<bool, sizeof...(T)>{ std::is_unbounded_array_v<T>... })
            {
                if(not is_dynamic)
                {
                    fixed_seen = true;
                }
                else if(fixed_seen)
                {
                    return false;
                }
            }
            return true;
        }();
    }

    template<bool IsMutable, class... T>
    class frame_view
    {
        static_assert(detail::dynamic_array_prefix<T...>, "dynamic arrays must form a frame prefix");

    public:
        using byte_type = maybe_mutable<IsMutable, unsigned char>;

        static consteval size_t size() noexcept
        {
            return sizeof...(T);
        }

        template<size_t I>
        constexpr decltype(auto) get() const noexcept
        {
            using type = maybe_mutable<
                IsMutable,
                typename type_list<T...>::template type_at<I>
            >;
            if constexpr (not std::is_unbounded_array_v<type>)
            {
                return *reinterpret_cast<type*>(end_ - offsets_to_end[I]);
            }
            else
            {
                using element_type = std::remove_extent_t<type>;
                const auto& span = *reinterpret_cast<const detail::dynamic_array_span*>(end_ - offsets_to_end[I]);
                return std::span<element_type>{ reinterpret_cast<element_type*>(end_ - span.offset_to_frame_end), span.count };
            }
        }

        template<size_t I>
        friend constexpr decltype(auto) get(frame_view self) noexcept
        {
            return self.template get<I>();
        }

    private:
        friend class frame_stack;

        static constexpr auto offsets_to_end = detail::offsets_to_end(std::array{ detail::layout_info_of<T>... });

        constexpr explicit frame_view(byte_type* end) noexcept
        : end_{ end }
        {}

        constexpr byte_type* aligned_begin() const
        {
            if constexpr (std::is_unbounded_array_v<typename type_list<T...>::template type_at<0>>)
            {
                const auto& span = *reinterpret_cast<const detail::dynamic_array_span*>(end_ - offsets_to_end[0]);
                return end_ - span.offset_to_frame_end;
            }
            else
            {
                constexpr size_t aligned_size = detail::align(offsets_to_end[0], max_alignment);
                return end_ - aligned_size;
            }
        }

        byte_type* end_;
    };

    namespace detail
    {
        template<class T, class TIniter>
        struct dynamic_array_initer
        {
            using type = T;
            using initer_type = TIniter;
            TIniter initer;

            constexpr stack_count_t count() const
            {
                if constexpr (std::unsigned_integral<std::remove_reference_t<TIniter>>)
                {
                    return (stack_count_t)initer;
                }
                else
                {
                    return (stack_count_t)std::ranges::distance(initer);
                }
            }
        };

        template<class T>
        concept is_dynamic_array_initer = std::same_as<
            std::remove_cvref_t<T>,
            dynamic_array_initer<typename std::remove_cvref_t<T>::type, typename std::remove_cvref_t<T>::initer_type>
        >;

        template<class T>
        struct stack_push_element : std::type_identity<T>{};

        template<class T, class TIniter>
        struct stack_push_element<dynamic_array_initer<T, TIniter>> : std::type_identity<T[]>{};

        template<class T>
        using stack_push_element_t = typename stack_push_element<std::remove_cvref_t<T>>::type;

        template<class T>
        struct dynamic_array_fn
        {
            template<class TIniter>
            requires (std::unsigned_integral<std::remove_cvref_t<TIniter>>
                || requires{
                    requires std::same_as<std::remove_const_t<std::ranges::range_value_t<TIniter>>, T>;
                })
            constexpr auto operator()(TIniter&& initer) const
            {
                return dynamic_array_initer<T, std::remove_const_t<TIniter>>{
                    (TIniter&&)initer
                };
            }
        };
    }

    template<class T>
    inline constexpr detail::dynamic_array_fn<T> dynamic_array{};

    template<class...T>
    struct frame_t : type_list<T...>{};

    template<class...T>
    inline constexpr frame_t<T...> frame{};

    class frame_stack
    {
    public:
        constexpr frame_stack() noexcept
        : begin_{ nullptr }
        , top_{ nullptr }
        , end_{ nullptr }
        {}

        constexpr explicit frame_stack(size_t capacity)
        {
            begin_ = (unsigned char*)::operator new[](capacity, std::align_val_t{ max_alignment });
            top_ = begin_;
            end_ = begin_ + capacity;
        }

        constexpr frame_stack(const frame_stack& other)
        {
            if(other.begin_ == nullptr) [[unlikely]]
            {
                begin_ = top_ = end_ = nullptr;
                return;
            }
            begin_ = (unsigned char*)::operator new[](other.capacity(), std::align_val_t{ max_alignment });
            std::memcpy(begin_, other.begin_, other.size());
            top_ = begin_ + other.size();
            end_ = begin_ + other.capacity();
        }

        constexpr frame_stack(frame_stack&& other) noexcept : frame_stack{}
        {
            swap(*this, other);
        }

        constexpr frame_stack& operator=(frame_stack other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        constexpr ~frame_stack()
        {
            ::operator delete[](begin_, std::align_val_t{ max_alignment });
        }

        friend constexpr void swap(frame_stack& left, frame_stack& right) noexcept
        {
            std::ranges::swap(left.begin_, right.begin_);
            std::ranges::swap(left.top_, right.top_);
            std::ranges::swap(left.end_, right.end_);
        }

        constexpr size_t size() const noexcept
        {
            return top_ - begin_;
        }

        constexpr bool empty() const noexcept
        {
            return size() == 0;
        }

        constexpr void clear() noexcept
        {
            top_ = begin_;
        }

        constexpr size_t capacity() const noexcept
        {
            return end_ - begin_;
        }

        constexpr unsigned char* data() noexcept
        {
            return begin_;
        }

        constexpr const unsigned char* data() const noexcept
        {
            return begin_;
        }

        constexpr void reserve(size_t new_capacity)
        {
            const size_t old_size = size();
            frame_stack new_stack{ old_size, new_capacity };
            if(old_size != 0) [[likely]]
            {
                std::memcpy(new_stack.begin_, begin_, old_size);
            }
            swap(*this, new_stack);
        }

        template<class... T>
        constexpr auto push()
        {
            constexpr size_t frame_size = detail::frame_size(std::array{ detail::layout_info_of<T>... });
            constexpr size_t increase_size = detail::align(frame_size, max_alignment);
            increase(increase_size);
            return frame_view<true, T...>{ top_ };
        }

        template<class...T>
        requires detail::dynamic_array_prefix<detail::stack_push_element_t<T>...>
        constexpr auto push(const T&... t)
        {
            constexpr size_t dynamic_count = std::ranges::count(
                std::array{ (bool)detail::is_dynamic_array_initer<T>... },
                true
            );
            constexpr size_t fixed_part = detail::frame_size(std::array{
                detail::layout_info_of<detail::stack_push_element_t<T>>...
            });

            auto spans = [&]<size_t... I>(std::index_sequence<I...>)
            {
                constexpr auto dynamic_element_layout_infos = std::array{
                    detail::layout_info_of<std::remove_extent_t<detail::stack_push_element_t<T>>>...
                };
                const auto tuple = std::forward_as_tuple(t...);

                return detail::caculate_dynamic_array_spans<
                    std::array<detail::layout_info, dynamic_count>{ dynamic_element_layout_infos[I]... }
                >(
                    fixed_part,
                    std::get<I>(tuple).count()...
                );
            }(std::make_index_sequence<dynamic_count>{});

            if constexpr(dynamic_count == 0)
            {
                increase(detail::align(fixed_part, max_alignment));
            }
            else
            {
                increase(spans[0].offset_to_frame_end);
                std::memcpy(top_ - fixed_part, spans.data(), spans.size() * sizeof(detail::dynamic_array_span));
            }

            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                const auto view = frame_view<true, detail::stack_push_element_t<T>...>{ top_ };
                (..., init_from(get<I>(view), (T&&)t));
                return view;
            }(std::index_sequence_for<T...>{});
        }

        template<class...T, class Self>
        constexpr auto top(this Self& self) noexcept
        {
            constexpr bool is_mutable = not std::is_const_v<Self>;

            return frame_view<is_mutable, T...>{ self.top_ };
        }

        template<frame_t First, frame_t...Rest, class Self>
        constexpr auto top(this Self& self) noexcept
        {
            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                if constexpr (sizeof...(Rest) == 0)
                {
                    return std::tuple{
                        self.template top<typename decltype(First)::template type_at<I>...>()
                    };
                }
                else
                {
                    const auto rest = self.template top<Rest...>();
                    const auto first = frame_view<not std::is_const_v<Self>, typename decltype(First)::template type_at<I>...>{
                        std::get<0>(rest).aligned_begin()
                    };
                    return std::tuple_cat(std::tuple{ first }, rest);
                }
            }(std::make_index_sequence<First.size()>{});
        }

        template<class... T>
        constexpr void pop() noexcept
        {
            top_ = frame_view<true, T...>{ top_ }.aligned_begin();

            GIVM_ASSERT(top_ >= begin_);
        }

        template<frame_t First, frame_t...Rest>
        constexpr void pop() noexcept
        {
            if constexpr (sizeof...(Rest) != 0)
            {
                pop<Rest...>();
            }
            [&]<size_t...I>(std::index_sequence<I...>)
            {
                top_ = frame_view<true, typename decltype(First)::template type_at<I>...>{ top_ }
                    .aligned_begin();
            }(std::make_index_sequence<First.size()>{});

            GIVM_ASSERT(top_ >= begin_);
        }

        template<bool IsMutable, class...T, class...Rest>
        constexpr void pop(frame_view<IsMutable, T...> first_frame, Rest...rest_frames) noexcept
        {
            top_ = first_frame.aligned_begin();
            GIVM_ASSERT(top_ >= begin_);
        }

    private:
        constexpr explicit frame_stack(size_t size, size_t capacity)
        {
            begin_ = (unsigned char*)::operator new[](capacity, std::align_val_t{ max_alignment });
            top_ = begin_ + size;
            end_ = begin_ + capacity;
        }

        constexpr void increase(size_t increase_size)
        {
            if(increase_size <= end_ - top_) [[likely]]
            {
                top_ += increase_size;
                return;
            }
            const size_t old_size = size();
            const size_t new_size = old_size + increase_size;
            frame_stack new_stack{ new_size, std::bit_ceil(new_size) };
            if(old_size != 0) [[likely]]
            {
                std::memcpy(new_stack.begin_, begin_, old_size);
            }
            swap(*this, new_stack);
        }

        template<class TTarget, class TSource>
        static constexpr void init_from(TTarget&& target, const TSource& source)
        {
            if constexpr (not detail::is_dynamic_array_initer<TSource>)
            {
                std::memcpy(&target, &source, sizeof(source));
            }
            else if constexpr (std::unsigned_integral<std::remove_reference_t<typename TSource::initer_type>>)
            {
                return;
            }
            else if constexpr (std::ranges::contiguous_range<typename TSource::initer_type>)
            {
                if(not std::ranges::empty(source.initer)) [[likely]]
                {
                    std::memcpy(target.data(), std::ranges::data(source.initer), target.size_bytes());
                }
            }
            else for(auto&& [i, elememt] : source.initer | std::views::enumerate)
            {
                typename TSource::type copy = elememt;
                std::memcpy(target.data() + i, &copy, sizeof(copy));
            }
        }

        unsigned char* begin_;
        unsigned char* top_;
        unsigned char* end_;
    };
}

template<bool IsMutable, class... T>
struct std::tuple_size<givm::frame_view<IsMutable, T...>>
    : std::integral_constant<size_t, sizeof...(T)>{};

template<size_t I, bool IsMutable, class... T>
struct std::tuple_element<I, givm::frame_view<IsMutable, T...>>
{
    using object_type = typename givm::type_list<T...>::template type_at<I>;
    using orig_type = givm::maybe_mutable<IsMutable, object_type>;
    using type = std::conditional_t<std::is_unbounded_array_v<orig_type>,
        std::span<std::remove_extent_t<orig_type>>,
        orig_type&
    >;
};

#include "../macro_undef.hpp"
#endif
