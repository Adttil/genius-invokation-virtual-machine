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

    class frame_stack;

    struct substack_t {};

    namespace detail
    {
        struct dynamic_array_span
        {
            size_t offset_to_frame_end;
            stack_count_t count;
        };

        template<class Frame>
        class substack_view;

        template<bool IsMutable, bool AtTop, class... T>
        class stack_frame_view;

        constexpr size_t align(size_t base, size_t alignment) noexcept
        {
            return (base + alignment - 1) & ~(alignment - 1);
        }

        inline constexpr size_t substack_tail_size = align(sizeof(size_t), max_alignment);

        struct layout_info
        {
            size_t size;
            size_t alignment;
        };

        template<class T>
        inline constexpr layout_info layout_info_of = []
        {
            if constexpr (std::same_as<T, substack_t>)
            {
                // The child storage follows the ordinary frame prefix.
                return layout_info{ 0, 1 };
            }
            else if constexpr (std::is_unbounded_array_v<T>)
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

        template<class... T>
        inline constexpr bool has_substack = (std::same_as<T, substack_t> || ...);

        template<class... T>
        concept frame_elements = sizeof...(T) != 0 && dynamic_array_prefix<T...>
            && ((not std::is_array_v<T> || not std::same_as<std::remove_cv_t<std::remove_extent_t<T>>, substack_t>) && ...)
            && []
        {
            constexpr std::array markers{ std::same_as<T, substack_t>... };
            for(size_t i = 0; i + 1 < markers.size(); ++i)
            {
                if(markers[i]) return false;
            }
            return true;
        }();
    }

    template<bool IsMutable, class... T>
    class frame_view
    {
        static_assert(detail::frame_elements<T...>, "a frame requires an array prefix and at most one final substack");

    public:
        using byte_type = maybe_mutable<IsMutable, unsigned char>;

        static consteval size_t size() noexcept { return sizeof...(T); }

        template<size_t I>
        constexpr decltype(auto) get() const noexcept
        {
            using object_type = typename type_list<T...>::template type_at<I>;
            using type = maybe_mutable<IsMutable, object_type>;
            if constexpr (std::same_as<object_type, substack_t>)
            {
                return detail::substack_view<frame_view>{ *this };
            }
            else if constexpr (not std::is_unbounded_array_v<type>)
            {
                return *reinterpret_cast<type*>(prefix_end() - offsets_to_end[I]);
            }
            else
            {
                using element_type = std::remove_extent_t<type>;
                const auto end = prefix_end();
                const auto& span = *reinterpret_cast<const detail::dynamic_array_span*>(end - offsets_to_end[I]);
                return std::span<element_type>{ reinterpret_cast<element_type*>(end - span.offset_to_frame_end), span.count };
            }
        }

        template<size_t I>
        friend constexpr decltype(auto) get(frame_view self) noexcept { return self.template get<I>(); }

    private:
        friend class frame_stack;
        template<class>
        friend class detail::substack_view;
        template<bool, bool, class...>
        friend class detail::stack_frame_view;

        static constexpr bool is_mutable = IsMutable;
        static constexpr bool stack_relative = false;
        static constexpr bool at_top = false;
        static constexpr auto offsets_to_end = detail::offsets_to_end(std::array{ detail::layout_info_of<T>... });

        constexpr explicit frame_view(byte_type* end) noexcept : end_{ end } {}

        constexpr byte_type* prefix_end() const noexcept
        {
            if constexpr (detail::has_substack<T...>)
            {
                const auto end = end_ - detail::substack_tail_size;
                return end - *reinterpret_cast<const size_t*>(end);
            }
            else return end_;
        }

        constexpr byte_type* aligned_begin() const
        {
            const auto end = prefix_end();
            if constexpr (std::is_unbounded_array_v<typename type_list<T...>::template type_at<0>>)
            {
                const auto& span = *reinterpret_cast<const detail::dynamic_array_span*>(end - offsets_to_end[0]);
                return end - span.offset_to_frame_end;
            }
            else
            {
                constexpr size_t aligned_size = detail::align(offsets_to_end[0], max_alignment);
                return end - aligned_size;
            }
        }

        byte_type* end_;
    };

    namespace detail
    {
        template<bool IsMutable, bool AtTop, class... T>
        class stack_frame_view
        {
            static_assert(frame_elements<T...>, "a frame requires an array prefix and at most one final substack");

        public:
            using byte_type = maybe_mutable<IsMutable, unsigned char>;
            static consteval size_t size() noexcept { return sizeof...(T); }

            template<size_t I>
            constexpr decltype(auto) get() const noexcept
            {
                if constexpr (std::same_as<typename type_list<T...>::template type_at<I>, substack_t>)
                {
                    return substack_view<stack_frame_view>{ *this };
                }
                else return frame_view<IsMutable, T...>{ end() }.template get<I>();
            }

            template<size_t I>
            friend constexpr decltype(auto) get(stack_frame_view self) noexcept { return self.template get<I>(); }

        private:
            friend class givm::frame_stack;
            template<class>
            friend class substack_view;

            static constexpr bool is_mutable = IsMutable;
            static constexpr bool stack_relative = true;
            static constexpr bool at_top = AtTop;
            using stack_type = maybe_mutable<IsMutable, frame_stack>;

            constexpr explicit stack_frame_view(stack_type* stack) noexcept requires AtTop : stack_{ stack } {}
            constexpr stack_frame_view(stack_type* stack, size_t offset) noexcept requires (not AtTop) : stack_{ stack, offset } {}
            constexpr byte_type* end() const noexcept;
            constexpr byte_type* aligned_begin() const noexcept { return frame_view<IsMutable, T...>{ end() }.aligned_begin(); }

            std::conditional_t<AtTop, stack_type*, std::pair<stack_type*, size_t>> stack_;
        };
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

        struct substack_initer {};

        template<>
        struct stack_push_element<substack_initer> : std::type_identity<substack_t>{};

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

    constexpr auto substack() noexcept
    {
        return detail::substack_initer{};
    }

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
        requires (detail::frame_elements<T...> && (not std::is_unbounded_array_v<T> && ...))
        constexpr auto push()
        {
            return push_blank<false, T...>();
        }

        template<class...T>
        requires detail::frame_elements<detail::stack_push_element_t<T>...>
        constexpr auto push(const T&... t)
        {
            return push_values<false>(t...);
        }

        template<class...T, class Self>
        constexpr auto top(this Self& self) noexcept
        {
            if constexpr (detail::has_substack<T...>)
            {
                return detail::stack_frame_view<not std::is_const_v<Self>, true, T...>{ &self };
            }
            else return frame_view<not std::is_const_v<Self>, T...>{ self.top_ };
        }

        template<frame_t First, frame_t...Rest, class Self>
        constexpr auto top(this Self& self) noexcept
        {
            return top_frames<First, Rest...>(self.top_, &self);
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
            top_ = frames_begin<First, Rest...>(top_);
            GIVM_ASSERT(top_ >= begin_);
        }

        template<class First, class... Rest>
        requires requires(First first) { first.aligned_begin(); }
        constexpr void pop(First first_frame, Rest...) noexcept
        {
            top_ = first_frame.aligned_begin();
            GIVM_ASSERT(top_ >= begin_);
        }

    private:
        template<bool, bool, class...>
        friend class detail::stack_frame_view;

        template<class>
        friend class detail::substack_view;

        template<frame_t... Frames, class Byte, class Owner>
        static constexpr auto top_frames(Byte* end, Owner owner) noexcept
        {
            using frames = type_list<decltype(Frames)...>;
            constexpr size_t count = sizeof...(Frames);
            constexpr bool is_mutable = not std::is_const_v<Byte>;
            using last = typename frames::template type_at<count - 1>;
            constexpr bool relocatable = not std::same_as<Owner, std::nullptr_t>
                && std::same_as<typename last::template type_at<last::size() - 1>, substack_t>;
            const auto make_view = [&]<size_t I>(Byte* frame_end)
            {
                return [&]<class... T>(frame_t<T...>)
                {
                    static_assert(not std::same_as<Owner, std::nullptr_t> || not detail::has_substack<T...>);
                    if constexpr (relocatable && I == count - 1)
                    {
                        return detail::stack_frame_view<is_mutable, true, T...>{ owner };
                    }
                    else if constexpr (relocatable)
                    {
                        return detail::stack_frame_view<is_mutable, false, T...>{
                            owner, static_cast<size_t>(frame_end - owner->begin_)
                        };
                    }
                    else return frame_view<is_mutable, T...>{ frame_end };
                }(typename frames::template type_at<I>{});
            };
            if constexpr (count == 1)
            {
                return std::tuple{ make_view.template operator()<0>(end) };
            }
            else
            {
                std::array<Byte*, count> ends;
                ends[count - 1] = end;
                [&]<size_t... I>(std::index_sequence<I...>)
                {
                    ((ends[count - 2 - I] = [&]<class... T>(frame_t<T...>)
                    {
                        return frame_view<is_mutable, T...>{ ends[count - 1 - I] }.aligned_begin();
                    }(typename frames::template type_at<count - 1 - I>{})), ...);
                }(std::make_index_sequence<count - 1>{});
                return [&]<size_t... I>(std::index_sequence<I...>)
                {
                    return std::tuple{ make_view.template operator()<I>(ends[I])... };
                }(std::make_index_sequence<count>{});
            }
        }

        template<frame_t... Frames, class Byte>
        static constexpr Byte* frames_begin(Byte* end) noexcept
        {
            using frames = type_list<decltype(Frames)...>;
            [&]<size_t... I>(std::index_sequence<I...>)
            {
                ((end = [&]<class... T>(frame_t<T...>)
                {
                    return frame_view<not std::is_const_v<Byte>, T...>{ end }.aligned_begin();
                }(typename frames::template type_at<sizeof...(Frames) - 1 - I>{})), ...);
            }(std::make_index_sequence<sizeof...(Frames)>{});
            return end;
        }

        template<bool InSubstack, class... T>
        constexpr auto push_blank()
        {
            constexpr size_t frame_size = detail::frame_size(std::array{ detail::layout_info_of<T>... });
            constexpr size_t prefix_size = detail::align(frame_size, max_alignment);
            constexpr size_t footer_size = detail::has_substack<T...> ? detail::substack_tail_size : 0;
            increase<InSubstack>(prefix_size + footer_size);
            if constexpr (detail::has_substack<T...>)
            {
                std::construct_at(reinterpret_cast<size_t*>(top_ - detail::substack_tail_size), size_t{});
                return detail::stack_frame_view<true, true, T...>{ this };
            }
            else return frame_view<true, T...>{ top_ - (InSubstack ? detail::substack_tail_size : 0) };
        }

        template<bool InSubstack, class...T>
        constexpr auto push_values(const T&... t)
        {
            constexpr size_t dynamic_count = std::ranges::count(
                std::array{ (bool)detail::is_dynamic_array_initer<T>... },
                true
            );
            constexpr size_t fixed_part = detail::frame_size(std::array{
                detail::layout_info_of<detail::stack_push_element_t<T>>...
            });
            constexpr bool has_substack = detail::has_substack<detail::stack_push_element_t<T>...>;
            constexpr size_t footer_size = has_substack ? detail::substack_tail_size : 0;

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
                increase<InSubstack>(detail::align(fixed_part, max_alignment) + footer_size);
            }
            else
            {
                increase<InSubstack>(spans[0].offset_to_frame_end + footer_size);
                const auto prefix_end = top_ - (InSubstack ? detail::substack_tail_size : 0) - footer_size;
                std::memcpy(prefix_end - fixed_part, spans.data(), spans.size() * sizeof(detail::dynamic_array_span));
            }
            if constexpr (has_substack)
            {
                std::construct_at(reinterpret_cast<size_t*>(top_ - detail::substack_tail_size), size_t{});
            }

            return [&]<size_t...I>(std::index_sequence<I...>)
            {
                const auto view = [&]
                {
                    if constexpr (has_substack)
                    {
                        return detail::stack_frame_view<true, true, detail::stack_push_element_t<T>...>{ this };
                    }
                    else return frame_view<true, detail::stack_push_element_t<T>...>{ top_ - (InSubstack ? detail::substack_tail_size : 0) };
                }();
                (..., init_from(get<I>(view), t));
                return view;
            }(std::index_sequence_for<T...>{});
        }

        constexpr explicit frame_stack(size_t size, size_t capacity)
        {
            begin_ = (unsigned char*)::operator new[](capacity, std::align_val_t{ max_alignment });
            top_ = begin_ + size;
            end_ = begin_ + capacity;
        }

        template<bool InSubstack>
        constexpr void increase(size_t increase_size)
        {
            if(increase_size <= end_ - top_) [[likely]]
            {
                top_ += increase_size;
            }
            else
            {
                const size_t old_size = size();
                const size_t new_size = old_size + increase_size;
                frame_stack new_stack{ new_size, std::bit_ceil(new_size) };
                if(old_size != 0) [[likely]]
                {
                    std::memcpy(new_stack.begin_, begin_, old_size);
                }
                swap(*this, new_stack);
            }
            if constexpr (InSubstack)
            {
                const auto destination = top_ - detail::substack_tail_size;
                const auto new_size = *reinterpret_cast<const size_t*>(destination - increase_size) + increase_size;
                std::construct_at(reinterpret_cast<size_t*>(destination), new_size);
            }
        }

        template<class TTarget, class TSource>
        static constexpr void init_from(TTarget&& target, const TSource& source)
        {
            if constexpr (std::same_as<detail::stack_push_element_t<TSource>, substack_t>)
            {
                return;
            }
            else if constexpr (not detail::is_dynamic_array_initer<TSource>)
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

    namespace detail
    {
        template<bool IsMutable, bool AtTop, class... T>
        constexpr stack_frame_view<IsMutable, AtTop, T...>::byte_type*
        stack_frame_view<IsMutable, AtTop, T...>::end() const noexcept
        {
            if constexpr (AtTop) return stack_->top_;
            else return stack_.first->begin_ + stack_.second;
        }

        template<class Frame>
        class substack_view
        {
        public:
            constexpr size_t size() const noexcept
            {
                return *reinterpret_cast<const size_t*>(end());
            }

            constexpr bool empty() const noexcept
            {
                return size() == 0;
            }

            template<class... T>
            requires (Frame::is_mutable && Frame::at_top && frame_elements<T...>
                && not has_substack<T...> && (not std::is_unbounded_array_v<T> && ...))
            constexpr auto push() const
            {
                return frame_.stack_->template push_blank<true, T...>();
            }

            template<class... T>
            requires (Frame::is_mutable && Frame::at_top && frame_elements<stack_push_element_t<T>...>
                && not has_substack<stack_push_element_t<T>...>)
            constexpr auto push(const T&... values) const
            {
                return frame_.stack_->template push_values<true>(values...);
            }

            template<class... T>
            requires (not has_substack<T...>)
            constexpr auto top() const noexcept
            {
                return frame_view<Frame::is_mutable, T...>{ end() };
            }

            template<frame_t First, frame_t... Rest>
            constexpr auto top() const noexcept
            {
                return frame_stack::top_frames<First, Rest...>(end(), nullptr);
            }

            template<class... T>
            requires (Frame::is_mutable && Frame::at_top && not has_substack<T...>)
            constexpr void pop() const noexcept
            {
                pop_to(frame_view<true, T...>{ end() }.aligned_begin());
            }

            template<frame_t First, frame_t... Rest>
            requires (Frame::is_mutable && Frame::at_top)
            constexpr void pop() const noexcept
            {
                pop_to(frame_stack::frames_begin<First, Rest...>(end()));
            }

            template<class First, class... Rest>
            requires (Frame::is_mutable && Frame::at_top) && requires(First first) { first.aligned_begin(); }
            constexpr void pop(First first, Rest...) const noexcept
            {
                pop_to(first.aligned_begin());
            }

        private:
            template<bool, class...>
            friend class givm::frame_view;
            template<bool, bool, class...>
            friend class stack_frame_view;

            constexpr explicit substack_view(Frame frame) noexcept : frame_{ frame } {}

            constexpr Frame::byte_type* end() const noexcept
            {
                if constexpr (Frame::stack_relative) return frame_.end() - substack_tail_size;
                else return frame_.end_ - substack_tail_size;
            }

            constexpr void pop_to(unsigned char* new_end) const noexcept requires (Frame::is_mutable && Frame::at_top)
            {
                const auto previous_end = end();
                const size_t decrease_size = previous_end - new_end;
                const auto previous_size = *reinterpret_cast<const size_t*>(previous_end);
                GIVM_ASSERT(decrease_size <= previous_size);
                std::construct_at(reinterpret_cast<size_t*>(new_end), previous_size - decrease_size);
                frame_.stack_->top_ = new_end + substack_tail_size;
            }

            Frame frame_;
        };
    }
}

template<bool IsMutable, class... T>
struct std::tuple_size<givm::frame_view<IsMutable, T...>>
    : std::integral_constant<size_t, sizeof...(T)>{};

template<size_t I, bool IsMutable, class... T>
struct std::tuple_element<I, givm::frame_view<IsMutable, T...>>
{
    using type = decltype(std::declval<givm::frame_view<IsMutable, T...>>().template get<I>());
};

template<bool IsMutable, bool AtTop, class... T>
struct std::tuple_size<givm::detail::stack_frame_view<IsMutable, AtTop, T...>>
    : std::integral_constant<size_t, sizeof...(T)>{};

template<size_t I, bool IsMutable, bool AtTop, class... T>
struct std::tuple_element<I, givm::detail::stack_frame_view<IsMutable, AtTop, T...>>
{
    using type = decltype(std::declval<givm::detail::stack_frame_view<IsMutable, AtTop, T...>>().template get<I>());
};

#include "../macro_undef.hpp"
#endif
