#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/enums/weapon_type.hpp>
#include <givm/givm.hpp>

#include "../../table/test_definition_library.hpp"
#include "../test_character_source.hpp"

namespace
{
    constexpr givm::character_id equipped_character{ givm::player_id{ 0 }, 0 };

    struct attachment_log
    {
        std::vector<std::string> events;
        std::vector<std::string> responders;
        std::vector<std::string> cost_responders;
        std::vector<givm::attachment_id> removed;
        bool pause_after_removal = false;
        bool add_during_removal = false;
        bool nested_added = false;
    };

    struct attachment_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type
        {
            attachment_log* log;
            std::string_view name;
        };
        attachment_log* log;
        std::string_view source_name;
        std::vector<std::string_view> source_tags;

        std::string_view name() const { return source_name; }
        const auto& tags() const { return source_tags; }
        definition_type compile(givm::definition_compile_context&) const { return { log, source_name }; }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&, givm::before_action&, givm::handle_context&)
        {
            data.log->responders.emplace_back(data.name);
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::attachment_view&, givm::cost_of_card&, givm::handle_context&)
        {
            data.log->cost_responders.emplace_back(data.name);
            return {};
        }
        static givm::program_entry handle(const definition_type&, const givm::attachment_view& self, givm::attachment_removed& event, givm::handle_context&)
        {
            CHECK(event.attachment != self.id());
            return {};
        }
    };

    struct equipment_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            attachment_log* log;
            bool prepare_equipment;
            givm::program_entry initial;
            givm::program_entry pause;
            givm::program_entry nested;
        };
        attachment_log* log;
        bool prepare_equipment;

        std::string_view name() const { return "EquippableCharacter"; }
        auto tags() const { return std::array{ std::string_view{ "equipment_target" } }; }
        auto attachment_dependencies() const
        {
            return std::array<std::string_view, 7>{ "OrdinaryA", "OrdinaryB", "Weapon", "Artifact", "Talent", "Technique", "NestedWeapon" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto add = [&](std::string_view name, std::uint32_t count)
            {
                return givm::add_attachment{
                    .definition = context.resolve_id<givm::attachment_view>(name), .state = { count }
                };
            };
            return {
                log, prepare_equipment,
                context.add_program(std::tuple{
                    add("Weapon", 7), add("OrdinaryA", 1), add("Technique", 6),
                    add("Talent", 5), add("OrdinaryB", 2), add("Artifact", 3)
                }),
                context.add_program(std::tuple{ givm::replace_cards{ givm::player_id{ 0 } } }),
                context.add_program(std::tuple{ add("NestedWeapon", 11) })
            };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            givm::character_state result{ .max_health = 10, .health = 10 };
            result.allowed_weapon_types.set(givm::weapon_type::sword);
            return result;
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::action_phase_started&, givm::handle_context& context)
        {
            return data.prepare_equipment ? context.invoke(data.initial) : givm::program_entry{};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::attachment_removed& event, givm::handle_context& context)
        {
            const auto id = event.attachment;
            const auto attachment = context.table()[id];
            CHECK_FALSE(attachment.is_valid());
            CHECK(attachment.definition_id().is_valid());
            CHECK(attachment.character().id() == equipped_character);
            if(attachment.character().has(givm::equipment_type::weapon)) CHECK(attachment.character().get(givm::equipment_type::weapon).id() != id);
            data.log->events.push_back("left:" + std::to_string(attachment.state().count));
            data.log->removed.push_back(id);
            if(data.log->pause_after_removal)
            {
                data.log->pause_after_removal = false;
                return context.invoke(data.pause);
            }
            if(data.log->add_during_removal && not data.log->nested_added)
            {
                data.log->nested_added = true;
                return context.invoke(data.nested);
            }
            return {};
        }
    };

    struct equipment_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            bool remove;
            givm::program_entry effect;
            givm::definition_id<givm::attachment_view> equipment;
            givm::tag_id target_tag;
            std::array<givm::tag_id, 5> weapon_types;
        };
        bool remove = false;

        std::string_view name() const { return remove ? "RemoveWeaponCard" : "EquipWeaponCard"; }
        auto attachment_dependencies() const { return std::array{ std::string_view{ "NewWeapon" } }; }
        auto tag_dependencies() const
        {
            return std::array<std::string_view, 6>{ "equipment_target", "sword", "claymore", "polearm", "bow", "catalyst" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                remove,
                remove ? context.add_program(std::tuple{ givm::remove_attachment{} })
                    : context.add_program(std::tuple{ givm::add_attachment{} }),
                context.resolve_id<givm::attachment_view>("NewWeapon"), context.resolve_tag("equipment_target"),
                { context.resolve_tag("sword"), context.resolve_tag("claymore"), context.resolve_tag("polearm"),
                    context.resolve_tag("bow"), context.resolve_tag("catalyst") }
            };
        }
        static givm::target_validation query(const definition_type& data, const givm::card_target_validation& parameters)
        {
            if(parameters.target_count == 0) return givm::target_validation::valid_incomplete;
            if(parameters.target_count != 1) return givm::target_validation::invalid;
            const auto target = std::get_if<givm::character_id>(&parameters.targets[0]);
            if(target == nullptr || target->player_id != parameters.card.player().id()) return givm::target_validation::invalid;
            const auto character = parameters.table[*target];
            if(not character.is_valid() || character.state().health == 0) return givm::target_validation::invalid;
            if(data.remove) return character.has(givm::equipment_type::weapon) ? givm::target_validation::valid_complete : givm::target_validation::invalid;
            if(not parameters.library[character.definition_id()].has_tag(data.target_tag)) return givm::target_validation::invalid;
            constexpr std::array weapon_types{ givm::weapon_type::sword, givm::weapon_type::claymore,
                givm::weapon_type::polearm, givm::weapon_type::bow, givm::weapon_type::catalyst };
            for(std::size_t index = 0; index < weapon_types.size(); ++index)
                if(parameters.library[data.equipment].has_tag(data.weapon_types[index]))
                    return character.state().allowed_weapon_types[weapon_types[index]]
                        ? givm::target_validation::valid_complete : givm::target_validation::invalid;
            return givm::target_validation::invalid;
        }
        static givm::program_entry handle(const definition_type& data, const givm::hand_card_view&, givm::card_effect& event, givm::handle_context& context)
        {
            const auto target = std::get<givm::character_id>(event.targets[0]);
            if(data.remove) return context.invoke(data.effect, givm::attachment_removal{ context.table()[target].get(givm::equipment_type::weapon).id() });
            return context.invoke(data.effect, givm::attachment_addition{
                .target = target, .definition = data.equipment, .state = { 9 }
            });
        }
    };

    struct tagged_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { bool allowed; };
        std::string_view source_name;
        bool allowed;
        std::string_view name() const { return source_name; }
        auto tags() const { return std::array{ std::string_view{ "equipment_target" } }; }
        definition_type compile(givm::definition_compile_context&) const { return { allowed }; }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            givm::character_state result{ .max_health = 10, .health = 10 };
            result.allowed_weapon_types.set(givm::weapon_type::sword, data.allowed);
            return result;
        }
    };

    auto compile_equipment_scenario(givm::compile_mode mode, attachment_log& log, bool prepare_equipment)
    {
        givm::character_state untagged_state{ .max_health = 10, .health = 10 };
        untagged_state.allowed_weapon_types.set(givm::weapon_type::sword);
        return givm::test::compile_definitions_with_program(mode,
            std::tuple{
                givm::draw_cards{ .count = 2 }, givm::begin_action{}, givm::end_game{ givm::game_result::both_loss }
            }, std::tuple{},
            givm::test::with_passive_skill(equipment_character_source{ &log, prepare_equipment }), tagged_character_source{ "Blocked", false },
            tagged_character_source{ "Reserve", true }, givm::test::initialized_character_source{ "Untagged", untagged_state },
            equipment_card_source{}, equipment_card_source{ true },
            attachment_source{ &log, "OrdinaryA" }, attachment_source{ &log, "OrdinaryB" },
            attachment_source{ &log, "Weapon", { "weapon", "sword" } }, attachment_source{ &log, "Artifact", { "artifact" } },
            attachment_source{ &log, "Talent", { "talent" } }, attachment_source{ &log, "Technique", { "technique" } },
            attachment_source{ &log, "NewWeapon", { "weapon", "sword" } }, attachment_source{ &log, "NestedWeapon", { "weapon", "sword" } });
    }

    auto zero_random = []() -> std::uint32_t { return 0; };

    givm::execution_state advance(givm::executor& execution, const givm::definition_library& library, givm::table& table)
    {
        auto state = execution.step(library, table, zero_random);
        while(state == givm::execution_state::active_character_changed || state == givm::execution_state::action_started
            || state == givm::execution_state::round_end_declared)
            state = execution.step(library, table, zero_random);
        return state;
    }

    givm::table load_equipment_scenario(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
            { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
        load_deck(table, library, {
            .cards = { ids.get_id<givm::card_definition>("RemoveWeaponCard"), ids.get_id<givm::card_definition>("EquipWeaponCard") },
            .characters = { ids.get_id<givm::character_view>("EquippableCharacter"), ids.get_id<givm::character_view>("Reserve"),
                ids.get_id<givm::character_view>("Blocked"), ids.get_id<givm::character_view>("Untagged") }
        }, { .characters = { ids.get_id<givm::character_view>("Reserve") } });
        return table;
    }

    std::size_t card_index(const givm::execution_view<givm::execution_state::action_selection>& action, const givm::table& table, givm::definition_id<givm::card_definition> definition)
    {
        for(std::size_t index = 0; index < action.card_count(); ++index)
            if(table[action.card_id(index)].definition_id() == definition) return index;
        FAIL("expected card is missing from action candidates");
        return 0;
    }

    struct dynamic_attachment_log
    {
        std::uint32_t calls = 0;
        std::vector<givm::attachment_id> added;
        std::vector<givm::attachment_id> left;
    };

    struct dynamic_attachment_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type
        {
            dynamic_attachment_log* log;
            givm::definition_id<givm::attachment_view> attachment;
            givm::program_entry add;
            givm::program_entry remove;
        };
        dynamic_attachment_log* log;
        std::string_view name() const { return "DynamicAttachmentCharacter"; }
        auto attachment_dependencies() const { return std::array<std::string_view, 2>{ "DynamicAttachment", "DynamicWeapon" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                log, context.resolve_id<givm::attachment_view>("DynamicAttachment"),
                context.add_program(std::tuple{
                    givm::add_attachment{
                        .definition = context.resolve_id<givm::attachment_view>("DynamicWeapon"),
                        .state = { 17 }
                    }, givm::add_attachment{}, givm::add_attachment{}
                }),
                context.add_program(std::tuple{ givm::remove_attachment{} })
            };
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            givm::character_state result{ .max_health = 10, .health = 10 };
            result.allowed_weapon_types.set(givm::weapon_type::sword);
            return result;
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::test_event&, givm::handle_context& context)
        {
            const auto& table = context.table();
            givm::character_id target{};
            std::uint32_t greatest_health = 0;
            for(const auto character : table[givm::player_id{ 1 }].characters())
            {
                if(character.state().health > greatest_health)
                {
                    target = character.id();
                    greatest_health = character.state().health;
                }
            }
            if(data.log->calls++ == 0)
            {
                return context.invoke(data.add,
                    givm::attachment_addition{ .target = target, .definition = data.attachment, .state = { 3 } },
                    givm::attachment_addition{ .target = target, .definition = data.attachment, .state = { 5 } });
            }
            for(const auto attachment : table[equipped_character].attachments())
                data.log->added.push_back(attachment.id());
            for(const auto attachment : table[target].attachments())
                data.log->added.push_back(attachment.id());
            for(const auto attachment : table[target].attachments())
            {
                if(attachment.state().count == 3)
                    return context.invoke(data.remove, givm::attachment_removal{ attachment.id() });
            }
            FAIL("first dynamic attachment is missing");
            return {};
        }
        static givm::program_entry handle(const definition_type& data, const givm::character_view&, givm::attachment_removed& event, givm::handle_context& context)
        {
            CHECK_FALSE(context.table()[event.attachment].is_valid());
            data.log->left.push_back(event.attachment);
            return {};
        }
    };
}

TEST_CASE("equipment cards validate targets and resume replacement after the removal broadcast", "[attachment][play_card][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attachment_log log;
    const auto [library, ids] = compile_equipment_scenario(mode, log, true);
    auto table = load_equipment_scenario(library, ids);
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    const std::vector<std::string> initial_order{ "Weapon", "Artifact", "Talent", "Technique", "OrdinaryA", "OrdinaryB" };
    CHECK(log.responders == initial_order);
    const auto character = table[equipped_character];
    REQUIRE(character.has(givm::equipment_type::weapon));
    REQUIRE(character.has(givm::equipment_type::artifact));
    REQUIRE(character.has(givm::equipment_type::talent));
    REQUIRE(character.has(givm::equipment_type::technique));
    CHECK(character.get(givm::equipment_type::artifact).state().count == 3);
    CHECK(character.get(givm::equipment_type::talent).state().count == 5);
    CHECK(character.get(givm::equipment_type::technique).state().count == 6);
    const auto old_weapon = character.get(givm::equipment_type::weapon).id();
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    const auto equipment_index = card_index(action, table, ids.get_id<givm::card_definition>("EquipWeaponCard"));
    CHECK(action.card_targets_validate(library, table, equipment_index) == givm::target_validation::valid_incomplete);
    const std::array<givm::card_target_id, 1> targets{ equipped_character };
    CHECK(action.card_targets_validate(library, table, equipment_index, targets) == givm::target_validation::valid_complete);
    for(const auto invalid : std::array{ givm::character_id{ givm::player_id{ 0 }, 2 },
        givm::character_id{ givm::player_id{ 0 }, 3 }, givm::character_id{ givm::player_id{ 1 }, 0 } })
    {
        const std::array<givm::card_target_id, 1> invalid_targets{ invalid };
        CHECK(action.card_targets_validate(library, table, equipment_index, invalid_targets) == givm::target_validation::invalid);
    }
    CHECK(action.calculate_card_cost(library, table, equipment_index).requirement.dice_requirement.any == 0);
    CHECK(log.cost_responders == initial_order);
    CHECK(action.card_payment_validate(table, equipment_index, {}) == givm::card_payment_validation::valid);
    log.events.clear();
    log.responders.clear();
    log.pause_after_removal = true;
    action.play_card(equipment_index, {}, targets);
    REQUIRE(advance(execution, library, table) == givm::execution_state::card_selection);
    CHECK(log.events == std::vector<std::string>{ "left:7" });
    CHECK_FALSE(table[old_weapon].is_valid());
    CHECK_FALSE(table[equipped_character].has(givm::equipment_type::weapon));
    CHECK(table[old_weapon].state().count == 7);
    const auto paused_log = log;
    auto copied_execution = execution;
    auto copied_table = table;
    const auto resume = [&](givm::executor& branch, givm::table& branch_table)
    {
        log = paused_log;
        branch.view_in<givm::execution_state::card_selection>().select({});
        REQUIRE(advance(branch, library, branch_table) == givm::execution_state::action_selection);
        CHECK(log.events == std::vector<std::string>{ "left:7" });
        CHECK(log.responders == std::vector<std::string>{ "NewWeapon", "Artifact", "Talent", "Technique", "OrdinaryA", "OrdinaryB" });
        CHECK_FALSE(branch_table[old_weapon].is_valid());
        CHECK(branch_table[old_weapon].definition_id() == ids.get_id<givm::attachment_view>("Weapon"));
        CHECK(branch_table[old_weapon].state().count == 7);
        const auto new_weapon = branch_table[equipped_character].get(givm::equipment_type::weapon).id();
        CHECK(new_weapon != old_weapon);
        CHECK(branch_table[new_weapon].definition_id() == ids.get_id<givm::attachment_view>("NewWeapon"));
        const auto next_action = branch.view_in<givm::execution_state::action_selection>();
        const auto removal_index = card_index(next_action, branch_table, ids.get_id<givm::card_definition>("RemoveWeaponCard"));
        CHECK(next_action.card_targets_validate(library, branch_table, removal_index, targets) == givm::target_validation::valid_complete);
        log.events.clear();
        next_action.play_card(library, branch_table, removal_index, {}, targets);
        REQUIRE(advance(branch, library, branch_table) == givm::execution_state::action_selection);
        CHECK(log.events == std::vector<std::string>{ "left:9" });
        CHECK_FALSE(branch_table[equipped_character].has(givm::equipment_type::weapon));
        CHECK_FALSE(branch_table[new_weapon].is_valid());
        CHECK(branch_table[new_weapon].state().count == 9);
        branch.view_in<givm::execution_state::action_selection>().declare_round_end();
        REQUIRE(advance(branch, library, branch_table) == givm::execution_state::action_selection);
        branch.view_in<givm::execution_state::action_selection>().declare_round_end();
        REQUIRE(advance(branch, library, branch_table) == givm::execution_state::finished);
        branch_table.clean_up();
        CHECK_FALSE(branch_table[equipped_character].has(givm::equipment_type::weapon));
        CHECK(branch_table[equipped_character].get(givm::equipment_type::artifact).definition_id() == ids.get_id<givm::attachment_view>("Artifact"));
        CHECK(branch_table[equipped_character].get(givm::equipment_type::talent).state().count == 5);
        CHECK(branch_table[equipped_character].get(givm::equipment_type::technique).state().count == 6);
        std::size_t count = 0;
        for(const auto attachment : branch_table[equipped_character].attachments())
        {
            CHECK(attachment.is_valid());
            ++count;
        }
        CHECK(count == 5);
    };
    resume(execution, table);
    resume(copied_execution, copied_table);
}

TEST_CASE("equipment replacement handles a nested replacement in the removal broadcast", "[attachment][broadcast][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attachment_log log;
    const auto [library, ids] = compile_equipment_scenario(mode, log, true);
    auto table = load_equipment_scenario(library, ids);
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    log.events.clear();
    log.add_during_removal = true;
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    const auto index = card_index(action, table, ids.get_id<givm::card_definition>("EquipWeaponCard"));
    const std::array<givm::card_target_id, 1> targets{ equipped_character };
    action.play_card(library, table, index, {}, targets);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    CHECK(log.events == std::vector<std::string>{ "left:7", "left:11" });
    REQUIRE(log.removed.size() == 2);
    CHECK_FALSE(table[log.removed[0]].is_valid());
    CHECK_FALSE(table[log.removed[1]].is_valid());
    CHECK(table[log.removed[1]].definition_id() == ids.get_id<givm::attachment_view>("NestedWeapon"));
    CHECK(table[equipped_character].get(givm::equipment_type::weapon).definition_id() == ids.get_id<givm::attachment_view>("NewWeapon"));
    std::size_t weapons = 0;
    for(const auto attachment : table[equipped_character].attachments())
        weapons += library[attachment.definition_id()].has_tag(ids.get_tag_id("weapon"));
    CHECK(weapons == 1);
}

TEST_CASE("equipment card input selects a reserve character without changing the active character", "[attachment][play_card][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    attachment_log log;
    const auto [library, ids] = compile_equipment_scenario(mode, log, false);
    auto table = load_equipment_scenario(library, ids);
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    const auto action = execution.view_in<givm::execution_state::action_selection>();
    const auto index = card_index(action, table, ids.get_id<givm::card_definition>("EquipWeaponCard"));
    const givm::character_id reserve{ givm::player_id{ 0 }, 1 };
    const std::array<givm::card_target_id, 1> targets{ reserve };
    CHECK(action.card_targets_validate(library, table, index, targets) == givm::target_validation::valid_complete);
    action.play_card(library, table, index, {}, targets);
    REQUIRE(advance(execution, library, table) == givm::execution_state::action_selection);
    CHECK_FALSE(table[equipped_character].has(givm::equipment_type::weapon));
    REQUIRE(table[reserve].has(givm::equipment_type::weapon));
    CHECK(table[reserve].get(givm::equipment_type::weapon).character().id() == reserve);
    CHECK(table[reserve].get(givm::equipment_type::weapon).state().count == 9);
    CHECK(table[givm::player_id{ 0 }].state().active_character == equipped_character);
}

TEST_CASE("a non-card response supplies multiple attachment inputs and removes only the selected attachment", "[attachment][program-input][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    dynamic_attachment_log log;
    attachment_log equipment_log;
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{
            givm::test_command{}, givm::test_command{}, givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, givm::test::with_passive_skill(dynamic_attachment_character_source{ &log }),
        givm::test::initialized_character_source{ "LowerHealth", { .max_health = 10, .health = 3 } },
        givm::test::initialized_character_source{ "HigherHealth", { .max_health = 10, .health = 9 } },
        givm::test::named_definition_source<givm::attachment_view>{ "DynamicAttachment" },
        attachment_source{ &equipment_log, "DynamicWeapon", { "weapon", "sword" } });
    givm::table table{ { .self_player = givm::player_id{ 0 } }, { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>("DynamicAttachmentCharacter") } },
        { .characters = { ids.get_id<givm::character_view>("LowerHealth"), ids.get_id<givm::character_view>("HigherHealth") } });
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(advance(execution, library, table) == givm::execution_state::finished);
    CHECK(log.calls == 2);
    REQUIRE(log.added.size() == 3);
    CHECK(library[table[log.added[0]].definition_id()].has_tag(ids.get_tag_id("weapon")));
    CHECK(table[equipped_character].get(givm::equipment_type::weapon).id() == log.added[0]);
    CHECK(table[equipped_character].get(givm::equipment_type::weapon).state().count == 17);
    const givm::character_id selected{ givm::player_id{ 1 }, 1 };
    CHECK(table[log.added[1]].character().id() == selected);
    CHECK(table[log.added[2]].character().id() == selected);
    CHECK(table[log.added[1]].state().count == 3);
    CHECK(table[log.added[2]].state().count == 5);
    CHECK(table[log.added[1]].definition_id() == table[log.added[2]].definition_id());
    CHECK_FALSE(table[log.added[1]].is_valid());
    CHECK(table[log.added[2]].is_valid());
    CHECK_FALSE(table[selected].has(givm::equipment_type::weapon));
    CHECK(log.left == std::vector{ log.added[1] });
    std::size_t remaining = 0;
    for(const auto attachment : table[selected].attachments())
    {
        CHECK(attachment.id() == log.added[2]);
        ++remaining;
    }
    CHECK(remaining == 1);
}
