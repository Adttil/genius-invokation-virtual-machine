#include "../test_source_library.hpp"

#include <array>
#include <concepts>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/givm.hpp>

#include "../table/test_definition_library.hpp"

namespace
{
    struct character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::definition_id<givm::skill_view> skill; };
        std::string_view source_name;
        std::string_view skill_name;

        std::string_view name() const { return source_name; }
        auto skill_dependencies() const { return std::array{ skill_name }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { context.resolve_id<givm::skill_view>(skill_name) };
        }
        static givm::definition_id<givm::skill_view> query(const definition_type& data, const givm::character_initial_skill& input)
        {
            return input.skill_index == 0 ? data.skill : givm::definition_id<givm::skill_view>{};
        }
        static givm::character_state query(const definition_type&, const givm::character_initial_state&)
        {
            return { .max_health = 10, .health = 10 };
        }
        static givm::program_entry handle(const definition_type&, const givm::character_view&, givm::before_action&, givm::handle_context&)
        {
            FAIL_CHECK("Global broadcasts do not dispatch to the character definition itself");
            return {};
        }
    };

    struct response_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            std::vector<givm::character_id>* handlers;
            givm::program_entry entry;
        };
        std::string_view source_name;
        std::vector<givm::character_id>* handlers;
        bool terminal;
        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            if(terminal)
                return { handlers, context.add_program(std::tuple{
                    givm::end_game{ .result = givm::game_result::player_1_win }
                }) };
            return { handlers, context.add_program(std::tuple{
                givm::set_element_aura{ .target = givm::character_id{ givm::player_id{ 0 }, 0 }, .aura = givm::element_aura::hydro }
            }) };
        }
        static givm::program_entry handle(
            const definition_type& data, const givm::skill_view& self, givm::test_event&,
            givm::handle_context& context)
        {
            if(not data.handlers->empty())
                CHECK(context.table()[givm::character_id{ givm::player_id{ 0 }, 0 }].state().aura == givm::element_aura::hydro);
            data.handlers->push_back(self.character().id());
            return context.invoke(data.entry);
        }
    };
    struct nested_selection_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            std::vector<givm::hand_card_id>* drawn;
            givm::program_entry draw_entry;
            givm::program_entry selection_entry;
        };
        std::vector<givm::hand_card_id>* drawn;

        std::string_view name() const noexcept { return "NestedSelection"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return {
                drawn,
                context.add_program(std::tuple{ givm::draw_cards{ .count = 2 } }),
                context.add_program(std::tuple{
                    givm::replace_cards{ .player = givm::player_id{ 0 } },
                    givm::start_round{ .max_rounds = 10 }
                })
            };
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::skill_view&, givm::test_event&,
            givm::handle_context& context)
        {
            return context.invoke(data.draw_entry);
        }

        static givm::program_entry handle(
            const definition_type& data, const givm::skill_view&, givm::card_drawn& event,
            givm::handle_context& context)
        {
            data.drawn->push_back(event.card);
            return context.invoke(data.selection_entry, std::span<const unsigned char>{});
        }
    };
    struct context_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            std::vector<std::uint32_t>* samples;
            givm::program_entry entry;
        };
        std::vector<std::uint32_t>* samples;

        std::string_view name() const noexcept { return "HandleContext"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { samples, context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                           givm::test_event&, givm::handle_context& context)
        {
            CHECK(context.table()[self.player().id()].deck_card_count() == 1);
            data.samples->push_back(context.random());
            data.samples->push_back(context.random());
            return context.invoke(data.entry);
        }
    };

    struct mixed_response_source
    {
        using definition_category = givm::skill_view;
        struct definition_type
        {
            std::vector<givm::character_id>* handlers;
            std::uint32_t* nested_responses;
            givm::program_entry empty;
            givm::program_entry change_active;
            givm::program_entry draw;
        };
        std::vector<givm::character_id>* handlers;
        std::uint32_t* nested_responses;

        std::string_view name() const noexcept { return "MixedResponses"; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { handlers, nested_responses,
                context.add_program(std::tuple{}),
                context.add_program(std::tuple{ givm::set_active_character{} }),
                context.add_program(std::tuple{ givm::draw_cards{ .count = 1 } }) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view& self,
                                          givm::test_event&, givm::handle_context& context)
        {
            data.handlers->push_back(self.character().id());
            switch(self.character().id().index)
            {
            case 0: return {};
            case 1: return context.invoke(data.empty);
            case 2: return context.invoke(data.change_active, givm::active_character_changed{ .current = self.character().id() });
            default: return context.invoke(data.draw);
            }
        }
        static givm::program_entry handle(const definition_type& data, const givm::skill_view&,
                                          givm::card_drawn&, givm::handle_context& context)
        {
            ++*data.nested_responses;
            return context.invoke(data.empty, std::span<const unsigned char>{});
        }
    };

    template<class TView>
    struct order_source
    {
        using definition_category = TView;
        struct definition_type
        {
            std::vector<std::string>* log;
            std::string_view name;
        };
        std::vector<std::string>* log;
        std::string_view source_name;
        std::vector<std::string_view> source_tags{};

        std::string_view name() const { return source_name; }
        const auto& tags() const { return source_tags; }
        definition_type compile(givm::definition_compile_context&) const { return { log, source_name }; }
        static givm::program_entry handle(const definition_type& data, const TView& self, givm::before_action&, givm::handle_context&)
        {
            std::string label = std::to_string(self.player().id().index) + ":" + std::string{ data.name };
            if constexpr(requires { self.character(); }) label += ":" + std::to_string(self.character().id().index);
            data.log->push_back(std::move(label));
            return {};
        }
    };

    struct order_card_source
    {
        using definition_category = givm::card_definition;
        struct definition_type
        {
            std::vector<std::string>* log;
            givm::program_entry prepare;
        };
        std::vector<std::string>* log;

        std::string_view name() const { return "OrderCard"; }
        auto attachment_dependencies() const
        {
            return std::array<std::string_view, 6>{ "OrdinaryA", "OrdinaryB", "Weapon", "Artifact", "Talent", "Technique" };
        }
        auto summon_dependencies() const { return std::array{ std::string_view{ "Summon" } }; }
        auto combat_status_dependencies() const { return std::array{ std::string_view{ "CombatStatus" } }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto attachments = [&](givm::relative_player player)
            {
                const auto add = [&](std::string_view name)
                {
                    return givm::add_attachment{ .player = player,
                        .definition = context.resolve_id<givm::attachment_view>(name), .state = { 1 } };
                };
                return std::tuple{ add("OrdinaryA"), add("Talent"), add("Weapon"), add("Technique"), add("OrdinaryB"), add("Artifact") };
            };
            return { log, context.add_program(std::tuple_cat(
                std::tuple{
                    givm::set_active_character{ .target = { givm::player_id{ 0 }, 1 } },
                    givm::set_active_character{ .target = { givm::player_id{ 1 }, 2 } }
                },
                attachments(givm::relative_player::current), attachments(givm::relative_player::other),
                std::tuple{
                    givm::add_combat_status{ .definition = context.resolve_id<givm::combat_status_view>("CombatStatus"), .state = { 1 } },
                    givm::add_combat_status{ .player = givm::relative_player::other,
                        .definition = context.resolve_id<givm::combat_status_view>("CombatStatus"), .state = { 1 } },
                    givm::add_summon{ .definition = context.resolve_id<givm::summon_view>("Summon"), .state = { 1, 1 } },
                    givm::add_summon{ .player = givm::relative_player::other,
                        .definition = context.resolve_id<givm::summon_view>("Summon"), .state = { 1, 1 } }
                }
            )) };
        }
        static givm::program_entry handle(const definition_type& data, const givm::deck_card_view& self, givm::test_event&, givm::handle_context& context)
        {
            return self.id() == givm::deck_card_id{ givm::player_id{ 0 }, 0 } ? context.invoke(data.prepare) : givm::program_entry{};
        }
        template<class TCard>
        static givm::program_entry handle(const definition_type& data, const TCard& self, givm::before_action&, givm::handle_context&)
        {
            const auto zone = std::same_as<TCard, givm::hand_card_view> ? ":hand:" : ":deck:";
            data.log->push_back(std::to_string(self.player().id().index) + zone + std::to_string(self.id().index));
            return {};
        }
    };

    struct zero_random { std::uint32_t operator()() const noexcept { return 0; } };
}

TEST_CASE("handle context exposes the current table random source and invocation", "[broadcast][handle-context]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<std::uint32_t> samples;
    const context_source source{ &samples };
    const character_source character{ "ContextCharacter", source.name() };
    const givm::test::named_definition_source<givm::card_definition> card{ "ContextCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, character, card);
    givm::table table;
    load_deck(table, library, {
        .cards = { ids.get_id<givm::card_definition>(card.name()) },
        .characters = { ids.get_id<givm::character_view>(character.name()) }
    }, {});
    struct sequential_random
    {
        std::uint32_t next = 17;
        std::uint32_t operator()() noexcept { return next++; }
    } random;
    givm::executor execution;
    execution.enter_entry(library);
    REQUIRE(execution.step(library, table, random) == givm::execution_state::finished);
    CHECK(samples == std::vector<std::uint32_t>{ 17, 18 });
    CHECK(random.next == 19);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 1);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
}

TEST_CASE("broadcast responses finish before the next handler and may end the game", "[broadcast]")
{
    const bool observed = GENERATE(false, true);
    const bool terminal = GENERATE(false, true);
    std::vector<givm::character_id> handlers;
    const response_source first{ "First", &handlers, terminal };
    const response_source second{ "Second", &handlers, false };
    const character_source first_character{ "FirstCharacter", first.name() };
    const character_source second_character{ "SecondCharacter", second.name() };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(first, second, first_character, second_character));
    const auto [library, ids] = compile(sources,
        std::tuple{ givm::test_command{}, givm::end_game{ .result = givm::game_result::both_loss } }, std::tuple{}, observed ? givm::compile_mode::observed : givm::compile_mode::normal
    );
    givm::table table;
    load_deck(table, library, { .characters = {
        ids.get_id<givm::character_view>(first_character.name()), ids.get_id<givm::character_view>(second_character.name())
    } }, {});
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random)
        == givm::execution_state::finished);
    if(terminal)
    {
        CHECK(handlers == std::vector{ givm::character_id{ givm::player_id{ 0 }, 0 } });
        CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::player_1_win);
    }
    else
    {
        CHECK(handlers == std::vector{ givm::character_id{ givm::player_id{ 0 }, 0 }, givm::character_id{ givm::player_id{ 0 }, 1 } });
        CHECK(target.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
    }
}

TEST_CASE("nested input resumes after library copies and moves in both compile modes", "[broadcast][bytecode][compile-mode][raw-input]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<givm::hand_card_id> drawn;
    const nested_selection_source source{ &drawn };
    const character_source character{ "NestedCharacter", source.name() };
    const givm::test::named_definition_source<givm::card_definition> card{ "NestedCard" };
    auto [library, ids] = givm::test::compile_definitions_with_program(
        mode,
        std::tuple{
            givm::test_command{},
            givm::draw_cards{ .count = 1 },
            givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, source, character, card
    );
    givm::table table;
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    load_deck(table, library, {
        .cards = { card_id, card_id, card_id },
        .characters = { ids.get_id<givm::character_view>(character.name()) }
    }, {});
    givm::executor target;
    target.enter_entry(library);
    zero_random random;
    REQUIRE(target.step(library, table, random) == givm::execution_state::card_selection);
    REQUIRE(drawn.size() == 1);
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 2);

    auto copied_library = library;
    auto assigned_library = copied_library;
    assigned_library = library;
    auto moved_library = std::move(library);
    auto move_assigned_library = copied_library;
    auto assignment_source = copied_library;
    move_assigned_library = std::move(assignment_source);
    auto copied_target = target;
    auto copied_table = table;
    const auto prefix = drawn;
    const auto resume = [&](givm::executor& execution, givm::table& current_table,
                            const givm::definition_library& current_library)
    {
        drawn = prefix;
        for(std::uint32_t index = 0; index < 3; ++index)
        {
            REQUIRE(drawn.size() == index + 1);
            CHECK(execution.view_in<givm::execution_state::card_selection>().player() == givm::player_id{ 0 });
            execution.view_in<givm::execution_state::card_selection>().select({});
            auto state = execution.step(current_library, current_table, random);
            if(mode == givm::compile_mode::observed)
            {
                REQUIRE(state == givm::execution_state::round_started);
                CHECK(current_table.state().round_number == index + 1);
                state = execution.step(current_library, current_table, random);
            }
            REQUIRE(state == (index == 2 ? givm::execution_state::finished : givm::execution_state::card_selection));
        }
        CHECK(current_table.state().round_number == 3);
        CHECK(current_table[givm::player_id{ 0 }].hand_card_count() == 3);
        CHECK(current_table[givm::player_id{ 0 }].deck_card_count() == 0);
        CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
        std::vector<givm::hand_card_id> hand;
        for(const auto entity : current_table[givm::player_id{ 0 }].hand_cards()) hand.push_back(entity.id());
        CHECK(drawn == hand);
    };
    resume(target, table, moved_library);
    const auto first_events = drawn;
    for(const auto* current_library : { &copied_library, &assigned_library, &move_assigned_library })
    {
        auto execution_copy = copied_target;
        auto table_copy = copied_table;
        resume(execution_copy, table_copy, *current_library);
        CHECK(drawn == first_events);
    }
}

TEST_CASE("consecutive broadcasts mix missing empty and parameterized response programs", "[broadcast][program-input][compile-mode]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<givm::character_id> handlers;
    std::uint32_t nested_responses = 0;
    const mixed_response_source source{ &handlers, &nested_responses };
    const character_source character{ "MixedCharacter", source.name() };
    const givm::test::named_definition_source<givm::card_definition> card{ "MixedResponseCard" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::test_command{}, givm::draw_cards{ .count = 1 },
                    givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, source, character, card);
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    givm::table table;
    load_deck(table, library, {
        .cards = { card_id, card_id, card_id },
        .characters = { character_id, character_id, character_id, character_id }
    }, {});
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    auto state = execution.step(library, table, random);
    while(state == givm::execution_state::active_character_changed)
        state = execution.step(library, table, random);
    REQUIRE(state == givm::execution_state::finished);
    CHECK(handlers == std::vector<givm::character_id>{
        { givm::player_id{ 0 }, 0 }, { givm::player_id{ 0 }, 1 },
        { givm::player_id{ 0 }, 2 }, { givm::player_id{ 0 }, 3 },
        { givm::player_id{ 0 }, 2 }, { givm::player_id{ 0 }, 3 },
        { givm::player_id{ 0 }, 0 }, { givm::player_id{ 0 }, 1 }
    });
    CHECK(nested_responses == 12);
    CHECK(table[givm::player_id{ 0 }].state().active_character == givm::character_id{ givm::player_id{ 0 }, 2 });
    CHECK(table[givm::player_id{ 0 }].hand_card_count() == 3);
    CHECK(table[givm::player_id{ 0 }].deck_card_count() == 0);
    CHECK(execution.view_in<givm::execution_state::finished>().result() == givm::game_result::both_loss);
}

TEST_CASE("global broadcasts follow acting player cyclic character and equipment order", "[broadcast][order]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<std::string> log;
    const order_source<givm::skill_view> skill{ &log, "Skill" };
    const character_source character{ "OrderCharacter", skill.name() };
    const order_card_source card{ &log };
    const order_source<givm::attachment_view> ordinary_a{ &log, "OrdinaryA" };
    const order_source<givm::attachment_view> ordinary_b{ &log, "OrdinaryB" };
    const order_source<givm::attachment_view> weapon{ &log, "Weapon", { "weapon" } };
    const order_source<givm::attachment_view> artifact{ &log, "Artifact", { "artifact" } };
    const order_source<givm::attachment_view> talent{ &log, "Talent", { "talent" } };
    const order_source<givm::attachment_view> technique{ &log, "Technique", { "technique" } };
    const order_source<givm::combat_status_view> status{ &log, "CombatStatus" };
    const order_source<givm::summon_view> summon{ &log, "Summon" };
    const auto [library, ids] = givm::test::compile_definitions_with_program(mode,
        std::tuple{ givm::test_command{}, givm::draw_cards{ .count = 1 },
            givm::draw_cards{ .count = 1, .player = givm::relative_player::other },
            givm::begin_action{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{},
        skill, character, card, ordinary_a, ordinary_b, weapon, artifact, talent, technique, status, summon);
    const auto character_id = ids.get_id<givm::character_view>(character.name());
    const auto card_id = ids.get_id<givm::card_definition>(card.name());
    const givm::linked_deck deck{
        .cards = { card_id, card_id, card_id }, .characters = { character_id, character_id, character_id }
    };
    givm::table table;
    load_deck(table, library, deck, deck);
    givm::executor execution;
    execution.enter_entry(library);
    zero_random random;
    const auto next_selection = [&]
    {
        auto state = execution.step(library, table, random);
        while(state != givm::execution_state::action_selection)
        {
            REQUIRE((state == givm::execution_state::active_character_changed
                || state == givm::execution_state::action_started || state == givm::execution_state::round_end_declared));
            state = execution.step(library, table, random);
        }
    };
    const std::vector<std::string> first{
        "0:Skill:1", "0:Weapon:1", "0:Artifact:1", "0:Talent:1", "0:Technique:1", "0:OrdinaryA:1", "0:OrdinaryB:1",
        "0:CombatStatus", "0:Skill:2", "0:Skill:0", "0:Summon", "0:hand:0", "0:deck:0", "0:deck:1"
    };
    const std::vector<std::string> second{
        "1:Skill:2", "1:Weapon:2", "1:Artifact:2", "1:Talent:2", "1:Technique:2", "1:OrdinaryA:2", "1:OrdinaryB:2",
        "1:CombatStatus", "1:Skill:0", "1:Skill:1", "1:Summon", "1:hand:0", "1:deck:0", "1:deck:1"
    };
    auto expected = first;
    expected.insert(expected.end(), second.begin(), second.end());
    next_selection();
    CHECK(log == expected);
    CHECK(table.state().active_player == givm::player_id{ 0 });
    log.clear();
    execution.view_in<givm::execution_state::action_selection>().declare_round_end();
    next_selection();
    expected = second;
    expected.insert(expected.end(), first.begin(), first.end());
    CHECK(log == expected);
    CHECK(table.state().active_player == givm::player_id{ 1 });
}
