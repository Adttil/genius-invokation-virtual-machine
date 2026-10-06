#ifndef GIVM_BASIC_DEFINITIONS_HPP
#define GIVM_BASIC_DEFINITIONS_HPP

#include "definition_source.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <tuple>
#include <variant>

namespace givm::genshin_impact
{
    struct shield_3_3_0_source
    {
        using definition_category = combat_status_view;

        struct definition_type
        {
            program_entry modify;
            program_entry remove;
        };

        constexpr std::string_view name() const noexcept
        {
            return "shield-3.3.0-genshin_impact";
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                context.add_program(std::tuple{ modify_combat_status_state{} }),
                context.add_program(std::tuple{ remove_combat_status{} })
            };
        }

        static constexpr combat_status_state query(const definition_type&, const combat_status_state_limit&) noexcept
        {
            return { .count = 2, .round_usages = 0 };
        }

        static program_entry handle(
            const definition_type& definition,
            damage_effect& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            if(event.type == damage_type::piercing || event.value == 0 || status.state().count == 0
                || status.player().state().active_character != event.target)
                return {};
            const auto absorbed = std::min(status.state().count, event.value);
            event.value -= absorbed;
            return context.invoke(definition.modify,
                modify_combat_status_state_input{ .status = status.id(), .count = -static_cast<std::int64_t>(absorbed) });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_regeneration& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            return context.invoke(definition.modify,
                modify_combat_status_state_input{ .status = status.id(), .count = event.state.count });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_state_changed& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, remove_combat_status_input{ status.id() });
        }
    };

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
            const definition_type& definition,
            damage_calculation& event, handle_context<attachment_view>& context, std::uint32_t = 0)
        {
            const auto attachment = context.entity();
            if(event.target != attachment.character().id()
                || (event.type != damage_type::physical && event.type != damage_type::pyro))
                return {};
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
            event.value = event.value > maximum - 2 ? maximum : event.value + 2;
            return context.invoke(definition.remove, remove_attachment_input{ attachment.id() });
        }

        static program_entry handle(
            const definition_type& definition,
            round_started&, handle_context<attachment_view>& context, std::uint32_t = 0)
        {
            const auto attachment = context.entity();
            return context.invoke(definition.remove, remove_attachment_input{ attachment.id() });
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
            const definition_type& definition,
            damage_calculation& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
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
                modify_combat_status_state_input{ .status = status.id(), .count = -1 });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_regeneration& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            return context.invoke(definition.refresh,
                set_combat_status_state_input{ .status = status.id(), .state = event.state });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_state_changed& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, remove_combat_status_input{ status.id() });
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
            const definition_type& definition,
            damage_calculation& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
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
                modify_combat_status_state_input{ .status = status.id(), .count = -1 });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_regeneration& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            return context.invoke(definition.refresh,
                set_combat_status_state_input{ .status = status.id(), .state = event.state });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_state_changed& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, remove_combat_status_input{ status.id() });
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
            const definition_type& definition,
            damage_calculation& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
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
                modify_combat_status_state_input{ .status = status.id(), .count = -1 });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_regeneration& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            return context.invoke(definition.refresh,
                set_combat_status_state_input{ .status = status.id(), .state = event.state });
        }

        static program_entry handle(
            const definition_type& definition,
            combat_status_state_changed& event, handle_context<combat_status_view>& context, std::uint32_t = 0)
        {
            const auto status = context.entity();
            if(event.current.count != 0)
                return {};
            return context.invoke(definition.remove, remove_combat_status_input{ status.id() });
        }
    };

    struct burning_flame_3_3_0_source
    {
        using definition_category = summon_view;

        struct definition_type
        {
            program_entry end_phase;
            program_entry accumulate;
        };

        constexpr std::string_view name() const noexcept
        {
            return "burning_flame-3.3.0-genshin_impact";
        }

        constexpr std::array<std::string_view, 1> tags() const noexcept
        {
            return { "remove_at_zero_usages" };
        }

        definition_type compile(definition_compile_context& context) const
        {
            return {
                context.add_program(std::tuple{ deal_damage{}, settle{}, modify_summon_state{} }),
                context.add_program(std::tuple{ modify_summon_state{} })
            };
        }

        static constexpr summon_state query(
            const definition_type&, const summon_state_limit&) noexcept
        {
            return { .value = 1, .usages = 2 };
        }

        static program_entry handle(
            const definition_type& definition,
            resummoning& event, handle_context<summon_view>& context, std::uint32_t = 0)
        {
            const auto summon = context.entity();
            return context.invoke(definition.accumulate,
                modify_summon_state_input{ .summons = std::array{ summon.id() }, .usages = event.state.usages });
        }

        static program_entry handle(
            const definition_type& definition,
            round_ended&, handle_context<summon_view>& context, std::uint32_t = 0)
        {
            const auto summon = context.entity();
            if(summon.state().usages == 0)
                return {};
            return context.invoke(definition.end_phase,
                deal_damage_input{ std::array{ damage{
                    .source = summon.id(),
                    .target = relative_character_target{ relative_player::opponent },
                    .value = summon.state().value,
                    .type = damage_type::pyro,
                    .flags = damage_flag_bits::combat_damage
                } } },
                modify_summon_state_input{ .summons = std::array{ summon.id() }, .usages = -1 });
        }
    };

    inline constexpr shield_3_3_0_source shield_3_3_0;
    inline constexpr frozen_3_3_0_source frozen_3_3_0;
    inline constexpr dendro_core_3_3_0_source dendro_core_3_3_0;
    inline constexpr catalyzing_field_3_3_0_source catalyzing_field_3_3_0;
    inline constexpr catalyzing_field_3_4_0_source catalyzing_field_3_4_0;
    inline constexpr burning_flame_3_3_0_source burning_flame_3_3_0;

    template<elemental_reaction Slot>
    struct reaction_3_3_0_source
    {
        using definition_category = reaction_view;

        struct definition_type
        {
            program_entry effect;
            definition_id<attachment_view> frozen;
        };

        std::string_view source_name;
        constexpr std::string_view name() const noexcept { return source_name; }

        constexpr auto combat_status_dependencies() const noexcept
        {
            if constexpr(Slot == elemental_reaction::bloom) return std::array{ dendro_core_3_3_0.name() };
            else if constexpr(Slot == elemental_reaction::quicken) return std::array{ catalyzing_field_3_4_0.name() };
            else if constexpr(Slot >= elemental_reaction::crystallize_cryo) return std::array{ shield_3_3_0.name() };
            else return std::array<std::string_view, 0>{};
        }

        constexpr auto summon_dependencies() const noexcept
        {
            if constexpr(Slot == elemental_reaction::burning) return std::array{ burning_flame_3_3_0.name() };
            else return std::array<std::string_view, 0>{};
        }

        constexpr auto attachment_dependencies() const noexcept
        {
            if constexpr(Slot == elemental_reaction::frozen) return std::array{ frozen_3_3_0.name() };
            else return std::array<std::string_view, 0>{};
        }

        definition_type compile(definition_compile_context& context) const
        {
            if constexpr(Slot == elemental_reaction::bloom || Slot == elemental_reaction::quicken)
            {
                const auto definition = context.resolve_id<combat_status_view>(combat_status_dependencies()[0]);
                return { context.add_program(std::tuple{ generate_combat_status{ .definition = definition,
                    .state = { Slot == elemental_reaction::bloom ? 1u : 2u, 0 } } }), {} };
            }
            else if constexpr(Slot == elemental_reaction::burning)
            {
                const auto definition = context.resolve_id<summon_view>(burning_flame_3_3_0.name());
                return { context.add_program(std::tuple{ summon{ .definition = definition, .state = { 1, 1 } } }), {} };
            }
            else if constexpr(Slot >= elemental_reaction::crystallize_cryo)
            {
                const auto definition = context.resolve_id<combat_status_view>(shield_3_3_0.name());
                return { context.add_program(std::tuple{ generate_combat_status{ .definition = definition, .state = { 1, 0 } } }), {} };
            }
            else if constexpr(Slot == elemental_reaction::frozen)
                return { context.add_program(std::tuple{ attach{} }), context.resolve_id<attachment_view>(frozen_3_3_0.name()) };
            else if constexpr(Slot == elemental_reaction::overloaded)
                return { context.add_program(std::tuple{ set_active_character{} }), {} };
            else if constexpr(Slot == elemental_reaction::superconduct || Slot == elemental_reaction::electro_charged
                || (Slot >= elemental_reaction::swirl_cryo && Slot <= elemental_reaction::swirl_electro))
                return { context.add_program(std::tuple{ deal_damage{} }), {} };
            else return {};
        }

        static program_entry handle(const definition_type&, damage_calculation& event,
            handle_context<reaction_view>&, std::uint32_t = 0)
        {
            constexpr std::uint32_t bonus = Slot == elemental_reaction::melt || Slot == elemental_reaction::vaporize
                || Slot == elemental_reaction::overloaded ? 2
                : Slot >= elemental_reaction::swirl_cryo && Slot <= elemental_reaction::swirl_electro ? 0 : 1;
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
            event.value = maximum - event.value < bonus ? maximum : event.value + bonus;
            return {};
        }

        static program_entry handle(const definition_type& definition, elemental_reaction_will_occur& event,
            handle_context<reaction_view>& context, std::uint32_t = 0)
        {
            if constexpr(Slot == elemental_reaction::superconduct || Slot == elemental_reaction::electro_charged
                || (Slot >= elemental_reaction::swirl_cryo && Slot <= elemental_reaction::swirl_electro))
            {
                constexpr auto type = Slot >= elemental_reaction::swirl_cryo && Slot <= elemental_reaction::swirl_electro
                    ? static_cast<damage_type>(static_cast<std::size_t>(Slot) - static_cast<std::size_t>(elemental_reaction::swirl_cryo))
                    : damage_type::piercing;
                return context.invoke(definition.effect, deal_damage_input{ std::array{ damage{ .source = event.source,
                    .target = event.target, .selection = character_selection::others, .value = 1,
                    .type = type, .flags = damage_flag_bits::reaction_damage } } });
            }
            else if constexpr(Slot == elemental_reaction::frozen)
            {
                if(context.table()[event.target].state().health == 0) return {};
                return context.invoke(definition.effect, attach_input{ event.target, definition.frozen });
            }
            else if constexpr(Slot == elemental_reaction::overloaded)
            {
                const auto player = context.table()[event.target.player_id];
                if(player.state().active_character != event.target) return {};
                const auto all = player.characters<false>();
                for(std::size_t offset = 1; offset < all.size(); ++offset)
                {
                    const auto target = all[(event.target.index + offset) % all.size()];
                    if(target && target.state().alive && target.state().health != 0)
                        return context.invoke(definition.effect, set_active_character_input{ target.id() });
                }
                return {};
            }
            else if constexpr(Slot == elemental_reaction::bloom || Slot == elemental_reaction::quicken
                || Slot == elemental_reaction::burning || Slot >= elemental_reaction::crystallize_cryo)
                return context.invoke(definition.effect);
            else return {};
        }
    };

    inline constexpr reaction_3_3_0_source<elemental_reaction::melt> melt_reaction_3_3_0{ "melt-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::vaporize> vaporize_reaction_3_3_0{ "vaporize-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::overloaded> overloaded_reaction_3_3_0{ "overloaded-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::superconduct> superconduct_reaction_3_3_0{ "superconduct-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::electro_charged> electro_charged_reaction_3_3_0{ "electro_charged-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::frozen> frozen_reaction_3_3_0{ "frozen-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::burning> burning_reaction_3_3_0{ "burning-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::bloom> bloom_reaction_3_3_0{ "bloom-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::quicken> quicken_reaction_3_3_0{ "quicken-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::swirl_cryo> swirl_cryo_reaction_3_3_0{ "swirl_cryo-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::swirl_hydro> swirl_hydro_reaction_3_3_0{ "swirl_hydro-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::swirl_pyro> swirl_pyro_reaction_3_3_0{ "swirl_pyro-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::swirl_electro> swirl_electro_reaction_3_3_0{ "swirl_electro-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::crystallize_cryo> crystallize_cryo_reaction_3_3_0{ "crystallize_cryo-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::crystallize_hydro> crystallize_hydro_reaction_3_3_0{ "crystallize_hydro-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::crystallize_pyro> crystallize_pyro_reaction_3_3_0{ "crystallize_pyro-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_3_3_0_source<elemental_reaction::crystallize_electro> crystallize_electro_reaction_3_3_0{ "crystallize_electro-reaction-3.3.0-genshin_impact" };

    inline constexpr reaction_definition_names reaction_names_3_3_0 = []
    {
        reaction_definition_names names;
        names[elemental_reaction::melt] = melt_reaction_3_3_0.name();
        names[elemental_reaction::vaporize] = vaporize_reaction_3_3_0.name();
        names[elemental_reaction::overloaded] = overloaded_reaction_3_3_0.name();
        names[elemental_reaction::superconduct] = superconduct_reaction_3_3_0.name();
        names[elemental_reaction::electro_charged] = electro_charged_reaction_3_3_0.name();
        names[elemental_reaction::frozen] = frozen_reaction_3_3_0.name();
        names[elemental_reaction::burning] = burning_reaction_3_3_0.name();
        names[elemental_reaction::bloom] = bloom_reaction_3_3_0.name();
        names[elemental_reaction::quicken] = quicken_reaction_3_3_0.name();
        names[elemental_reaction::swirl_cryo] = swirl_cryo_reaction_3_3_0.name();
        names[elemental_reaction::swirl_hydro] = swirl_hydro_reaction_3_3_0.name();
        names[elemental_reaction::swirl_pyro] = swirl_pyro_reaction_3_3_0.name();
        names[elemental_reaction::swirl_electro] = swirl_electro_reaction_3_3_0.name();
        names[elemental_reaction::crystallize_cryo] = crystallize_cryo_reaction_3_3_0.name();
        names[elemental_reaction::crystallize_hydro] = crystallize_hydro_reaction_3_3_0.name();
        names[elemental_reaction::crystallize_pyro] = crystallize_pyro_reaction_3_3_0.name();
        names[elemental_reaction::crystallize_electro] = crystallize_electro_reaction_3_3_0.name();
        return names;
    }();

    inline definition_source_library reaction_sources_3_3_0()
    {
        definition_source_library result;
        (void)result.add(
            dendro_core_3_3_0,
            catalyzing_field_3_4_0,
            burning_flame_3_3_0,
            frozen_3_3_0,
            shield_3_3_0,
            melt_reaction_3_3_0,
            vaporize_reaction_3_3_0,
            overloaded_reaction_3_3_0,
            superconduct_reaction_3_3_0,
            electro_charged_reaction_3_3_0,
            frozen_reaction_3_3_0,
            burning_reaction_3_3_0,
            bloom_reaction_3_3_0,
            quicken_reaction_3_3_0,
            swirl_cryo_reaction_3_3_0,
            swirl_hydro_reaction_3_3_0,
            swirl_pyro_reaction_3_3_0,
            swirl_electro_reaction_3_3_0,
            crystallize_cryo_reaction_3_3_0,
            crystallize_hydro_reaction_3_3_0,
            crystallize_pyro_reaction_3_3_0,
            crystallize_electro_reaction_3_3_0
        );
        return result;
    }

}

#endif
