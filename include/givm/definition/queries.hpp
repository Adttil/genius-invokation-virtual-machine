#ifndef GIVM_DEFINITION_QUERIES_HPP
#define GIVM_DEFINITION_QUERIES_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "events.hpp"

namespace givm
{
    struct character_initial_state
    {
        using result_t = character_state;
    };

    struct character_initial_skill
    {
        using result_t = definition_id<skill_view>;

        std::size_t skill_index;
    };

    struct card_initial_cost
    {
        using result_t = action_cost_requirement;
    };

    struct skill_initial_cost
    {
        using result_t = action_cost_requirement;
    };

    enum class target_validation : std::uint8_t
    {
        invalid,
        valid_incomplete,
        valid_complete_or_continue,
        valid_complete
    };

    struct card_target_validation
    {
        using result_t = target_validation;

        hand_card_view card;
        const givm::table& table;
        std::array<card_target_id, 2> targets;
        std::size_t target_count;
    };

    struct skill_target_validation
    {
        using result_t = target_validation;

        skill_view skill;
        const givm::table& table;
        std::array<skill_target_id, 2> targets;
        std::size_t target_count;
    };

    constexpr character_state query_default(const character_initial_state&) noexcept
    {
        return {};
    }

    constexpr definition_id<skill_view> query_default(const character_initial_skill&) noexcept
    {
        return {};
    }

    constexpr action_cost_requirement query_default(const card_initial_cost&) noexcept
    {
        return { .dice_requirement = {}, .speed = action_speed::fast };
    }

    constexpr action_cost_requirement query_default(const skill_initial_cost&) noexcept
    {
        return {};
    }

    constexpr target_validation query_default(const card_target_validation& query) noexcept
    {
        return query.target_count == 0 ? target_validation::valid_complete : target_validation::invalid;
    }

    constexpr target_validation query_default(const skill_target_validation& query) noexcept
    {
        return query.target_count == 0 ? target_validation::valid_complete : target_validation::invalid;
    }
}

#endif
