#ifndef GIVM_TABLE_TABLE_STORAGE_HPP
#define GIVM_TABLE_TABLE_STORAGE_HPP

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#include "../enums/game_result.hpp"
#include "data/player_data.hpp"
#include "data/status_data.hpp"
#include "entity_id.hpp"
#include "history_summary.hpp"

namespace givm
{
    struct table_state
    {
        std::uint32_t round_number = 0;
        std::uint32_t max_rounds = 14;
        player_id active_player{ 0 };
        bool first_ended = false;
        player_id self_player{ 2 };
    };

    namespace detail
    {
        template<class T>
        struct history_allocator
        {
            using value_type = T;
            using is_always_equal = std::true_type;

            constexpr history_allocator() noexcept = default;
            template<class U>
            constexpr history_allocator(const history_allocator<U>&) noexcept {}

            T* allocate(std::size_t count)
            {
                if constexpr(std::is_same_v<T, unsigned char>)
                {
                    auto* data = static_cast<unsigned char*>(::operator new(
                        count, std::align_val_t{ alignof(std::max_align_t) }));
                    // Starting the byte array also implicitly creates the field objects.
                    return ::new(static_cast<void*>(data)) unsigned char[count];
                }
                else return std::allocator<T>{}.allocate(count);
            }

            void deallocate(T* data, std::size_t count) noexcept
            {
                if constexpr(std::is_same_v<T, unsigned char>)
                    ::operator delete(data, std::align_val_t{ alignof(std::max_align_t) });
                else std::allocator<T>{}.deallocate(data, count);
            }

            // allocate already started the byte lifetimes. Leave default-inserted
            // bytes untouched; copying their representations must not restart them.
            constexpr void construct(unsigned char*) noexcept requires std::is_same_v<T, unsigned char> {}
            constexpr void construct(unsigned char* data, unsigned char value) noexcept
                requires std::is_same_v<T, unsigned char> { *data = value; }
            constexpr void destroy(unsigned char*) noexcept requires std::is_same_v<T, unsigned char> {}
            template<class U>
            constexpr bool operator==(const history_allocator<U>&) const noexcept { return true; }
        };

        struct table_storage
        {
            table_state state;
            player_data player_datas[2]{};
            std::vector<status_slot> status_slots;
            std::vector<unsigned char, history_allocator<unsigned char>> history_summaries;
        };
    }
}

#endif
