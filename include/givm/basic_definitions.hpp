#ifndef GIVM_BASIC_DEFINITIONS_HPP
#define GIVM_BASIC_DEFINITIONS_HPP

#include "executor.hpp"

#include <array>
#include <limits>
#include <string_view>
#include <tuple>
#include <variant>

namespace givm::genshin_impact
{
    struct frozen_3_3_0_source
    {
        using definition_category = attachment_view;

        struct definition_type
        {
            program_entry remove;
        };

        constexpr std::string_view name() const noexcept
        {
            return "frozen-3.3.0-genshin_impact";
        }

        constexpr std::array<std::string_view, 1> tags() const noexcept
        {
            return { "control" };
        }

        definition_type compile(definition_compile_context& context) const
        {
            return { context.add_program(std::tuple{ remove_attachment{} }) };
        }

        static constexpr attachment_state query(const definition_type&, const attachment_state_limit&) noexcept
        {
            return { .count = 1, .round_usages = 0 };
        }

        static program_entry handle(
            const definition_type& definition, const attachment_view& attachment,
            damage_calculation& event, handle_context& context)
        {
            if(event.target != attachment.character().id()
                || (event.type != damage_type::physical && event.type != damage_type::pyro))
                return {};
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
            event.value = event.value > maximum - 2 ? maximum : event.value + 2;
            return context.invoke(definition.remove, attachment_removal{ attachment.id() });
        }

        static program_entry handle(
            const definition_type& definition, const attachment_view& attachment,
            round_started&, handle_context& context)
        {
            return context.invoke(definition.remove, attachment_removal{ attachment.id() });
        }
    };

    struct dendro_core_3_3_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type
        {
            program_entry consume;
            program_entry refresh;
            program_entry remove;
        };

        constexpr std::string_view name() const noexcept
        {
            return "dendro_core-3.3.0-genshin_impact";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                context.add_program(std::tuple{ modify_combat_status_state{} }),
                context.add_program(std::tuple{ set_combat_status_state{} }),
                context.add_program(std::tuple{ remove_combat_status{} })
            };
        }

        static constexpr combat_status_state query(
            const definition_type&, const combat_status_state_limit&) noexcept
        {
            return { .count = 1, .round_usages = 0 };
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            damage_calculation& event, handle_context& context)
        {
            if(status.state().count == 0
                || (event.type != damage_type::pyro && event.type != damage_type::electro))
                return {};
            const auto player = status.player().id();
            const auto opponent = other_player(player);
            if(event.target.player_id != opponent
                || context.table()[opponent].state().active_character != event.target)
                return {};
            const auto source_player = std::visit([](const auto source)
            {
                if constexpr(requires { source.player_id; })
                    return source.player_id;
                else if constexpr(requires { source.character_id; })
                    return source.character_id.player_id;
                else
                    return source.card_id.player_id;
            }, event.source);
            if(source_player != player)
                return {};

            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
            event.value = event.value > maximum - 2 ? maximum : event.value + 2;
            return context.invoke(definition.consume,
                combat_status_state_modification{ .status = status.id(), .count = -1 });
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            combat_status_regeneration& event, handle_context& context)
        {
            return context.invoke(definition.refresh,
                combat_status_state_change{ .status = status.id(), .state = event.state });
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            combat_status_state_changed& event, handle_context& context)
        {
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, combat_status_removal{ status.id() });
        }
    };

    struct catalyzing_field_3_3_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type
        {
            program_entry consume;
            program_entry refresh;
            program_entry remove;
        };

        constexpr std::string_view name() const noexcept
        {
            return "catalyzing_field-3.3.0-genshin_impact";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                context.add_program(std::tuple{ modify_combat_status_state{} }),
                context.add_program(std::tuple{ set_combat_status_state{} }),
                context.add_program(std::tuple{ remove_combat_status{} })
            };
        }

        static constexpr combat_status_state query(
            const definition_type&, const combat_status_state_limit&) noexcept
        {
            return { .count = 3, .round_usages = 0 };
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            damage_calculation& event, handle_context& context)
        {
            if(status.state().count == 0
                || (event.type != damage_type::electro && event.type != damage_type::dendro))
                return {};
            const auto player = status.player().id();
            const auto opponent = other_player(player);
            if(event.target.player_id != opponent
                || context.table()[opponent].state().active_character != event.target)
                return {};
            const auto source_player = std::visit([](const auto source)
            {
                if constexpr(requires { source.player_id; })
                    return source.player_id;
                else if constexpr(requires { source.character_id; })
                    return source.character_id.player_id;
                else
                    return source.card_id.player_id;
            }, event.source);
            if(source_player != player)
                return {};

            if(event.value != std::numeric_limits<std::uint32_t>::max())
                ++event.value;
            return context.invoke(definition.consume,
                combat_status_state_modification{ .status = status.id(), .count = -1 });
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            combat_status_regeneration& event, handle_context& context)
        {
            return context.invoke(definition.refresh,
                combat_status_state_change{ .status = status.id(), .state = event.state });
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            combat_status_state_changed& event, handle_context& context)
        {
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, combat_status_removal{ status.id() });
        }
    };

    struct catalyzing_field_3_4_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type
        {
            program_entry consume;
            program_entry refresh;
            program_entry remove;
        };

        constexpr std::string_view name() const noexcept
        {
            return "catalyzing_field-3.4.0-genshin_impact";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                context.add_program(std::tuple{ modify_combat_status_state{} }),
                context.add_program(std::tuple{ set_combat_status_state{} }),
                context.add_program(std::tuple{ remove_combat_status{} })
            };
        }

        static constexpr combat_status_state query(
            const definition_type&, const combat_status_state_limit&) noexcept
        {
            return { .count = 2, .round_usages = 0 };
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            damage_calculation& event, handle_context& context)
        {
            if(status.state().count == 0
                || (event.type != damage_type::electro && event.type != damage_type::dendro))
                return {};
            const auto player = status.player().id();
            const auto opponent = other_player(player);
            if(event.target.player_id != opponent
                || context.table()[opponent].state().active_character != event.target)
                return {};
            const auto source_player = std::visit([](const auto source)
            {
                if constexpr(requires { source.player_id; })
                    return source.player_id;
                else if constexpr(requires { source.character_id; })
                    return source.character_id.player_id;
                else
                    return source.card_id.player_id;
            }, event.source);
            if(source_player != player)
                return {};

            if(event.value != std::numeric_limits<std::uint32_t>::max())
                ++event.value;
            return context.invoke(definition.consume,
                combat_status_state_modification{ .status = status.id(), .count = -1 });
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            combat_status_regeneration& event, handle_context& context)
        {
            return context.invoke(definition.refresh,
                combat_status_state_change{ .status = status.id(), .state = event.state });
        }

        static program_entry handle(
            const definition_type& definition, const combat_status_view& status,
            combat_status_state_changed& event, handle_context& context)
        {
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, combat_status_removal{ status.id() });
        }
    };

    struct burning_flame_3_3_0_source
    {
        using definition_category = summon_view;

        struct definition_type
        {
            program_entry end_phase;
            program_entry accumulate;
            program_entry remove;
        };

        constexpr std::string_view name() const noexcept
        {
            return "burning_flame-3.3.0-genshin_impact";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                context.add_program(std::tuple{ deal_damage{}, modify_summon_state{} }),
                context.add_program(std::tuple{ modify_summon_state{} }),
                context.add_program(std::tuple{ remove_summon{} })
            };
        }

        static constexpr summon_state query(
            const definition_type&, const summon_state_limit&) noexcept
        {
            return { .value = 1, .usages = 2 };
        }

        static program_entry handle(
            const definition_type& definition, const summon_view& summon,
            resummoning& event, handle_context& context)
        {
            return context.invoke(definition.accumulate,
                summon_state_modification{ .summon = summon.id(), .usages = event.state.usages });
        }

        static program_entry handle(
            const definition_type& definition, const summon_view& summon,
            summon_state_changed& event, handle_context& context)
        {
            if(event.current.usages != 0)
                return {};
            return context.invoke(definition.remove, summon_removal{ summon.id() });
        }

        static program_entry handle(
            const definition_type& definition, const summon_view& summon,
            round_ended&, handle_context& context)
        {
            if(summon.state().usages == 0)
                return {};
            return context.invoke(definition.end_phase,
                damage{
                    .source = summon.id(),
                    .target = relative_character_target{ relative_player::opponent },
                    .value = summon.state().value,
                    .type = damage_type::pyro,
                    .flags = damage_flag_bits::combat_damage
                },
                summon_state_modification{ .summon = summon.id(), .usages = -1 });
        }
    };

    inline constexpr frozen_3_3_0_source frozen_3_3_0;
    inline constexpr dendro_core_3_3_0_source dendro_core_3_3_0;
    inline constexpr catalyzing_field_3_3_0_source catalyzing_field_3_3_0;
    inline constexpr catalyzing_field_3_4_0_source catalyzing_field_3_4_0;
    inline constexpr burning_flame_3_3_0_source burning_flame_3_3_0;
}

#endif
