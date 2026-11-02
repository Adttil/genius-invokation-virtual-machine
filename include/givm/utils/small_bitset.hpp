#ifndef GIVM_UTILS_SMALL_BITSET_HPP
#define GIVM_UTILS_SMALL_BITSET_HPP

#include <array>
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>

namespace givm
{
    template<std::size_t InlineBits>
    class small_bitset;

    class bitset_view
    {
    public:
        class iterator
        {
        public:
            using iterator_category = std::random_access_iterator_tag;
            using iterator_concept = std::random_access_iterator_tag;
            using value_type = bool;
            using difference_type = std::ptrdiff_t;
            using reference = bool;
            using pointer = void;

            constexpr iterator() noexcept = default;

            constexpr bool operator*() const noexcept
            {
                return (words_[index_ / 64] & (std::uint64_t{ 1 } << (index_ % 64))) != 0;
            }

            constexpr bool operator[](difference_type offset) const noexcept { return *(*this + offset); }

            constexpr iterator& operator++() noexcept { ++index_; return *this; }
            constexpr iterator& operator--() noexcept { --index_; return *this; }
            constexpr iterator operator++(int) noexcept { auto copy = *this; ++*this; return copy; }
            constexpr iterator operator--(int) noexcept { auto copy = *this; --*this; return copy; }

            constexpr iterator& operator+=(difference_type offset) noexcept { index_ += offset; return *this; }
            constexpr iterator& operator-=(difference_type offset) noexcept { index_ -= offset; return *this; }

            friend constexpr iterator operator+(iterator value, difference_type offset) noexcept
            {
                value += offset;
                return value;
            }

            friend constexpr iterator operator+(difference_type offset, iterator value) noexcept
            {
                return value + offset;
            }

            friend constexpr iterator operator-(iterator value, difference_type offset) noexcept
            {
                value -= offset;
                return value;
            }

            friend constexpr difference_type operator-(const iterator& first, const iterator& second) noexcept
            {
                assert(first.words_ == second.words_);
                return static_cast<difference_type>(first.index_ - second.index_);
            }

            friend constexpr bool operator==(const iterator&, const iterator&) noexcept = default;

            friend constexpr auto operator<=>(const iterator& first, const iterator& second) noexcept
            {
                assert(first.words_ == second.words_);
                return first.index_ <=> second.index_;
            }

        private:
            friend class bitset_view;
            constexpr iterator(const std::uint64_t* words, std::size_t index) noexcept : words_{ words }, index_{ index } {}

            const std::uint64_t* words_ = nullptr;
            std::size_t index_ = 0;
        };

        constexpr bitset_view() noexcept = default;

        constexpr std::size_t size() const noexcept { return size_; }
        constexpr bool empty() const noexcept { return size_ == 0; }
        constexpr iterator begin() const noexcept { return iterator{ words_, 0 }; }
        constexpr iterator end() const noexcept { return iterator{ words_, size_ }; }

        constexpr bool operator[](std::size_t index) const noexcept
        {
            assert(index < size_);
            return (words_[index / 64] & (std::uint64_t{ 1 } << (index % 64))) != 0;
        }

        constexpr bitset_view take(std::size_t count) const noexcept
        {
            assert(count <= size_);
            return bitset_view{ words_, count };
        }

    private:
        template<std::size_t> friend class small_bitset;

        constexpr bitset_view(const std::uint64_t* words, std::size_t size) noexcept
            : words_{ words }, size_{ size } {}

        const std::uint64_t* words_ = nullptr;
        std::size_t size_ = 0;
    };

    template<std::size_t InlineBits>
    class small_bitset
    {
        static_assert(InlineBits <= std::size_t(-1) / 64 * 64);
        static constexpr std::size_t inline_word_count = InlineBits / 64 + (InlineBits % 64 != 0);

    public:
        static constexpr std::size_t inline_capacity = inline_word_count * 64;

        class reference
        {
        public:
            constexpr reference(const reference&) noexcept = default;

            constexpr operator bool() const noexcept { return (*word_ & mask_) != 0; }

            constexpr reference& operator=(bool value) noexcept
            {
                if(value) *word_ |= mask_;
                else *word_ &= ~mask_;
                return *this;
            }

            constexpr reference& operator=(const reference& other) noexcept
            {
                return *this = static_cast<bool>(other);
            }

        private:
            friend class small_bitset;

            constexpr reference(std::uint64_t& word, std::size_t bit) noexcept
                : word_{ &word }, mask_{ std::uint64_t{ 1 } << bit } {}

            std::uint64_t* word_;
            std::uint64_t mask_;
        };

        constexpr small_bitset() noexcept = default;

        constexpr explicit small_bitset(std::size_t size) : size_{ size }
        {
            if(size_ > inline_capacity) storage_.heap = new std::uint64_t[word_count()]{};
        }

        constexpr small_bitset(const small_bitset& other) : storage_{ other.storage_ }, size_{ other.size_ }
        {
            if(size_ > inline_capacity)
            {
                storage_.heap = new std::uint64_t[word_count()];
                for(std::size_t index = 0; index < word_count(); ++index)
                    storage_.heap[index] = other.storage_.heap[index];
            }
        }

        constexpr small_bitset(small_bitset&& other) noexcept
        {
            swap(other);
        }

        constexpr small_bitset& operator=(small_bitset other) noexcept
        {
            swap(other);
            return *this;
        }

        constexpr void swap(small_bitset& other) noexcept
        {
            std::swap(storage_, other.storage_);
            std::swap(size_, other.size_);
        }

        friend constexpr void swap(small_bitset& first, small_bitset& second) noexcept
        {
            first.swap(second);
        }

        constexpr ~small_bitset()
        {
            if(size_ > inline_capacity) delete[] storage_.heap;
        }

        constexpr std::size_t size() const noexcept { return size_; }
        constexpr bool empty() const noexcept { return size_ == 0; }

        constexpr bool operator[](std::size_t index) const noexcept { return view()[index]; }

        constexpr reference operator[](std::size_t index) noexcept
        {
            assert(index < size_);
            return reference{ data()[index / 64], index % 64 };
        }

        constexpr void set(std::size_t index, bool value = true) noexcept { (*this)[index] = value; }

        constexpr void reset() noexcept
        {
            auto* words = data();
            for(std::size_t index = 0; index < word_count(); ++index) words[index] = 0;
        }

        constexpr bitset_view view() const noexcept { return bitset_view{ data(), size_ }; }
        constexpr bitset_view take(std::size_t count) const noexcept { return view().take(count); }

    private:
        constexpr std::size_t word_count() const noexcept { return size_ / 64 + (size_ % 64 != 0); }

        constexpr auto* data(this auto& self) noexcept
        {
            return self.size_ > inline_capacity ? self.storage_.heap : self.storage_.local.data();
        }

        union storage_type
        {
            std::array<std::uint64_t, inline_word_count> local;
            std::uint64_t* heap;

            constexpr storage_type() noexcept : local{} {}
        } storage_;
        std::size_t size_ = 0;
    };
}

#endif
