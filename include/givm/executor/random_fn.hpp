#ifndef GIVM_EXECUTOR_RANDOM_FN_HPP
#define GIVM_EXECUTOR_RANDOM_FN_HPP

#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <type_traits>

namespace givm
{
    class random_fn
    {
    public:
        template<class TRandom>
            requires std::invocable<TRandom&>
                && std::convertible_to<std::invoke_result_t<TRandom&>, std::uint32_t>
        constexpr explicit random_fn(TRandom& random) noexcept
        : object_{ std::addressof(random) }
        , invoke_{ +[](void* object) -> std::uint32_t
            {
                return static_cast<std::uint32_t>(std::invoke(*static_cast<TRandom*>(object)));
            }
        }
        {}

        std::uint32_t operator()() const
        {
            return invoke_(object_);
        }

    private:
        void* object_;
        std::uint32_t (*invoke_)(void*);
    };

    static_assert(std::is_trivially_copyable_v<random_fn>);
}

#endif
