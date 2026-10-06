#ifndef GIVM_DEFINITION_QUERIES_HPP
#define GIVM_DEFINITION_QUERIES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "events.hpp"

namespace givm
{
    class definition_library;

    struct character_initial_state
    {
        using result_t = character_state;
    };

    struct summon_state_limit
    {
        using result_t = summon_state;
    };

    struct support_state_limit
    {
        using result_t = support_state;
    };

    struct combat_status_state_limit
    {
        using result_t = combat_status_state;
    };

    struct attachment_state_limit
    {
        using result_t = attachment_state;
    };

    struct character_initial_skill
    {
        using result_t = definition_id<skill_view>;

        std::size_t skill_index;
    };

    struct character_reaction_override
    {
        using result_t = definition_id<reaction_view>;
        elemental_reaction slot;
    };

    struct reaction_aura
    {
        using result_t = element_aura;
        elemental_reaction slot;
        element_aura reacted_aura;
        element incoming_element;
    };

    struct card_initial_state
    {
        using result_t = card_state;
    };

    struct card_state_modification
    {
        using result_t = void;

        card_state& state;
        const status_state& status;
    };

    struct skill_initial_cost
    {
        using result_t = action_cost_requirement;
    };

    struct technique_initial_cost
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
        const definition_library& library;
        std::array<card_target_id, 2> targets;
        std::size_t target_count;
    };

    struct card_equipment_target_validation
    {
        using result_t = bool;

        hand_card_view card;
        character_view character;
    };

    struct skill_target_validation
    {
        using result_t = target_validation;

        skill_view skill;
        const givm::table& table;
        const definition_library& library;
        std::array<skill_target_id, 2> targets;
        std::size_t target_count;
    };

    struct technique_target_validation
    {
        using result_t = target_validation;

        attachment_view technique;
        const givm::table& table;
        const definition_library& library;
        std::array<technique_target_id, 2> targets;
        std::size_t target_count;
    };

    constexpr character_state query_default(const character_initial_state&) noexcept
    {
        return {};
    }

    constexpr summon_state query_default(const summon_state_limit&) noexcept
    {
        return { std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    }

    constexpr support_state query_default(const support_state_limit&) noexcept
    {
        return { std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    }

    constexpr combat_status_state query_default(const combat_status_state_limit&) noexcept
    {
        return { std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    }

    constexpr attachment_state query_default(const attachment_state_limit&) noexcept
    {
        return { std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::uint32_t>::max() };
    }

    constexpr definition_id<skill_view> query_default(const character_initial_skill&) noexcept
    {
        return {};
    }

    constexpr definition_id<reaction_view> query_default(const character_reaction_override&) noexcept
    {
        return {};
    }

    constexpr element_aura query_default(const reaction_aura& query) noexcept
    {
        return aura_after_reaction(query.reacted_aura, query.incoming_element, query.slot);
    }

    constexpr card_state query_default(const card_initial_state&) noexcept
    {
        return {};
    }

    constexpr void query_default(const card_state_modification&) noexcept
    {}

    constexpr action_cost_requirement query_default(const skill_initial_cost&) noexcept
    {
        return {};
    }

    constexpr target_validation query_default(const card_target_validation& query) noexcept
    {
        return query.target_count == 0 ? target_validation::valid_complete : target_validation::invalid;
    }

    constexpr bool query_default(const card_equipment_target_validation&) noexcept
    {
        return false;
    }

    constexpr target_validation query_default(const skill_target_validation& query) noexcept
    {
        return query.target_count == 0 ? target_validation::valid_complete : target_validation::invalid;
    }
    constexpr action_cost_requirement query_default(const technique_initial_cost&) noexcept
    {
        return {};
    }

    constexpr target_validation query_default(const technique_target_validation& query) noexcept
    {
        return query.target_count == 0 ? target_validation::valid_complete : target_validation::invalid;
    }

}

#endif
