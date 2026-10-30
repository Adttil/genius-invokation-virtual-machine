#include <array>
#include <ranges>
#include <tuple>
#include <type_traits>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/compile.hpp>
#include <givm/runtime.hpp>

#include "../table/test_definition_library.hpp"

namespace givm_test::executor::roles
{
    template<class T> concept reads_definition = requires(T value) { value.definition_id(); };
    template<class T> concept reads_dice = requires(T value) { value.dice(); };
    template<class T> concept reads_first_choice = requires(T value) { value.first_selected_character(); };
    template<class T> concept reads_cost = requires(T value, const givm::definition_library& library,
        const givm::table& table) { value.calculate_switch_cost(library, table, 0); };
    template<class T> concept submits_action = requires(T value, const givm::definition_library& library,
        givm::table& table, decltype(givm_test::zero_random)& random)
        { value.declare_round_end(library, table, random); };

    using own_action = givm::execution_view<givm::execution_state::action_selection0, givm::role::player0>;
    using other_action = givm::execution_view<givm::execution_state::action_selection0, givm::role::player1>;
    using spectator_action = givm::execution_view<givm::execution_state::action_selection0, givm::role::spectator>;
    static_assert(reads_cost<own_action> && submits_action<own_action>);
    static_assert(!reads_cost<other_action> && !submits_action<other_action>);
    static_assert(reads_cost<spectator_action> && !submits_action<spectator_action>);
    static_assert(sizeof(own_action) == sizeof(givm::execution_view<givm::execution_state::action_selection0>));
    static_assert(reads_dice<givm::perspective_player_view<0, givm::role::player0>>);
    static_assert(!reads_dice<givm::perspective_player_view<1, givm::role::player0>>);
    static_assert(reads_dice<givm::perspective_player_view<1, givm::role::spectator>>);
    static_assert(!reads_definition<givm::hidden_hand_card_view>);
    static_assert(!reads_first_choice<givm::execution_view<
        givm::execution_state::remaining_active_character_selection0, givm::role::player0>>);
    static_assert(reads_first_choice<givm::execution_view<
        givm::execution_state::remaining_active_character_selection0, givm::role::player1>>);

    struct tagged_card : givm::test::named_definition_source<givm::definition_category::card>
    {
        constexpr auto tags() const
        {
            return std::array{ std::string_view{ "a_private" }, std::string_view{ "z_public" } };
        }
    };

    TEST_CASE("compilation configures public tags independently of definition sources", "[role][tags][compile]")
    {
        const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
        const bool select_subset = GENERATE(false, true);
        const bool span_commands = GENERATE(false, true);
        const bool static_compile = GENERATE(false, true);
        CAPTURE(mode, select_subset, span_commands, static_compile);
        const tagged_card card{ { "ConfiguredTagCard" } }, other{ { "OtherTagCard" } };
        auto sources = givm_test::make_source_library();
        REQUIRE(sources.add(card, other));
        const std::array selected{ card.name() };
        givm::definition_selection selection{};
        selection[static_cast<std::size_t>(givm::definition_category::card)] = selected;
        const std::array public_tags{ std::string_view{ "z_public" }, std::string_view{ "unused" },
            std::string_view{ "z_public" } };
        const auto compile_program = [&](const auto& commands)
        {
            if(select_subset)
            {
                if(static_compile)
                    return givm::definition_library::compile(sources, givm_test::basic_sources,
                        selection, commands, commands, mode, public_tags);
                return givm::compile(sources, givm_test::basic_sources,
                    selection, commands, commands, mode, public_tags);
            }
            if(static_compile)
                return givm::definition_library::compile(sources, givm_test::basic_sources,
                    commands, commands, mode, public_tags);
            return givm::compile(sources, givm_test::basic_sources, commands, commands, mode, public_tags);
        };
        const auto [library, ids] = givm_test::require_success(span_commands
            ? compile_program(std::span<const givm::any_command>{}) : compile_program(std::tuple{}));
        const auto id = ids.get_id<givm::definition_category::card>(card.name());
        const auto public_tag = ids.get_tag_id("z_public");
        const auto private_tag = ids.get_tag_id("a_private");
        REQUIRE(library.public_tag_count() == 2);
        CHECK(ids.public_tag_count() == library.public_tag_count());
        CHECK(public_tag.value() < library.public_tag_count());
        CHECK(private_tag.value() >= library.public_tag_count());
        CHECK(library.has_tag(id, public_tag));
        CHECK(library.public_tags(id).has(public_tag));
        CHECK_FALSE(library.public_tags(id).has(private_tag));
        CHECK(ids.has<givm::definition_category::card>(other.name()) == !select_subset);
        CHECK(ids.has_tag("unused"));
        CHECK_FALSE(library.public_tags(id).has(ids.get_tag_id("unused")));

        const auto [private_library, private_ids] = givm_test::require_success(givm::compile(
            sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, mode));
        const auto private_id = private_ids.get_id<givm::definition_category::card>(card.name());
        CHECK(private_library.public_tag_count() == 0);
        CHECK(private_library.has_tag(private_id, private_ids.get_tag_id("z_public")));
        CHECK(private_library.public_tags(private_id).empty());
    }

    TEST_CASE("table roles expose public hand tags while preserving real card IDs", "[role][table][tags]")
    {
        const tagged_card card{ { "RoleCard" } };
        auto sources = givm_test::make_source_library();
        REQUIRE(sources.add(card));
        const std::array public_tags{ std::string_view{ "z_public" } };
        const auto [library, ids] = givm_test::require_success(givm::compile(
            sources, givm_test::basic_sources,
            std::tuple{ givm::draw_cards{ .position = 0 },
                givm::draw_cards{ .player = givm::relative_player::opponent, .position = 0 },
                givm::replace_cards_both{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss } },
            std::tuple{}, givm::compile_mode::normal, public_tags));
        const auto card_definition = ids.get_id<givm::definition_category::card>(card.name());
        const auto visible_tag = ids.get_tag_id("z_public");
        const auto private_tag = ids.get_tag_id("a_private");
        REQUIRE(library.public_tag_count() == 1);
        CHECK(ids.public_tag_count() == library.public_tag_count());
        CHECK(visible_tag.value() == 0);
        CHECK(private_tag.value() >= library.public_tag_count());
        CHECK(library.has_tag(card_definition, visible_tag));
        CHECK(library.has_tag(card_definition, private_tag));

        givm::player_state first, second;
        first.dice[givm::elemental_dice::pyro] = 2;
        second.dice[givm::elemental_dice::hydro] = 3;
        givm::table table{ { .self_player = givm::player_id{ 0 } }, first, second };
        const givm::linked_deck deck{ .cards = { card_definition } };
        load_deck(table, library, deck, deck);
        givm::executor execution;
        auto random = givm_test::zero_random;
        REQUIRE(execution.start(library, table).resume(library, table, random)
            == givm::execution_state::initial_card_selection);

        const auto view = table.view_for<givm::role::player0>();
        const auto [self, opponent] = view.players();
        CHECK(self.dice()[givm::elemental_dice::pyro] == 2);
        CHECK(table.view_for<givm::role::full>().player<1>().state().dice.total() == 3);
        CHECK(opponent.dice_count() == 3);
        REQUIRE(self.hand_card_count() == 1);
        REQUIRE(opponent.hand_card_count() == 1);
        const auto own = *self.hand_cards().begin();
        const auto hidden = *opponent.hand_cards().begin();
        CHECK(own.definition_id() == card_definition);
        CHECK(hidden.id().player_id() == givm::player_id{ 1 });
        CHECK(opponent[hidden.id()].id() == hidden.id());
        const auto statuses = hidden.statuses();
        CHECK(statuses.begin() == statuses.end());
        const auto tags = library.public_tags(hidden);
        CHECK(tags.size() == 1);
        CHECK(tags.has(visible_tag));
        CHECK_FALSE(tags.has(private_tag));
        CHECK(table.view_for<givm::role::spectator>().player<1>()[hidden.id()].definition_id() == card_definition);

        const auto remaining = execution.view_in<givm::execution_state::initial_card_selection,
            givm::role::player1>().select(library, table, random, {});
        REQUIRE(remaining == givm::execution_state::card_selection0);
        CHECK(execution.view_in<givm::execution_state::card_selection0>().player() == givm::player_id{ 0 });
        CHECK(execution.view_in<givm::execution_state::card_selection0, givm::role::player0>()
            .select(library, table, random, {}) == givm::execution_state::finished);
    }

    TEST_CASE("dice states distinguish both participants from either remaining player", "[role][selection][dice]")
    {
        const auto [library, ids] = givm::test::compile_definitions_with_program(
            givm::compile_mode::normal,
            std::tuple{ givm::start_dice_roll_phase{ .count = 2, .reroll_count = { 1, 1 } },
                givm::settle{}, givm::end_game{ givm::game_result::both_loss } }, std::tuple{});
        givm::table table;
        givm::executor execution;
        auto random = givm_test::zero_random;
        REQUIRE(execution.start(library, table).resume(library, table, random)
            == givm::execution_state::dice_selection_both);
        auto copied_execution = execution;
        auto copied_table = table;
        CHECK(execution.view_in<givm::execution_state::dice_selection_both, givm::role::player0>()
            .select(library, table, random, {}) == givm::execution_state::dice_selection1);
        CHECK(execution.view_in<givm::execution_state::dice_selection1, givm::role::player0>().player()
            == givm::player_id{ 1 });
        CHECK(execution.view_in<givm::execution_state::dice_selection1, givm::role::player1>()
            .select(library, table, random, {}) == givm::execution_state::finished);
        CHECK(copied_execution.view_in<givm::execution_state::dice_selection_both, givm::role::player1>()
            .select(library, copied_table, random, {}) == givm::execution_state::dice_selection0);
        CHECK(copied_execution.view_in<givm::execution_state::dice_selection0, givm::role::player0>()
            .select(library, copied_table, random, {}) == givm::execution_state::finished);
    }

    TEST_CASE("initial character roles bind input and keep the first choice private from the other player", "[role][selection][characters]")
    {
        const givm::test::initialized_character_source character{ "RoleCharacter" };
        const auto [library, ids] = givm::test::compile_definitions_with_program(
            givm::compile_mode::normal,
            std::tuple{ givm::select_active_character_both{}, givm::settle{},
                givm::end_game{ givm::game_result::both_loss } }, std::tuple{}, character);
        givm::table table;
        const givm::linked_deck deck{ .characters = {
            ids.get_id<givm::definition_category::character>(character.name()) } };
        load_deck(table, library, deck, deck);
        givm::executor execution;
        auto random = givm_test::zero_random;
        REQUIRE(execution.start(library, table).resume(library, table, random)
            == givm::execution_state::initial_active_character_selection);
        const auto first = execution.view_in<givm::execution_state::initial_active_character_selection,
            givm::role::player1>();
        const givm::character_id own{ givm::player_id{ 1 }, 0 }, other{ givm::player_id{ 0 }, 0 };
        CHECK(first.selection_validate(table, other) == givm::initial_active_character_selection_validation::wrong_player);
        REQUIRE(first.select(library, table, random, own)
            == givm::execution_state::remaining_active_character_selection0);
        CHECK(execution.view_in<givm::execution_state::remaining_active_character_selection0,
            givm::role::player1>().first_selected_character() == own);
        CHECK(execution.view_in<givm::execution_state::remaining_active_character_selection0,
            givm::role::spectator>().first_selected_character() == own);
        CHECK(execution.view_in<givm::execution_state::remaining_active_character_selection0,
            givm::role::player0>().select(library, table, random, other) == givm::execution_state::finished);
    }
}
