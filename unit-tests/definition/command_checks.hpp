#include <array>
#include <cstddef>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <variant>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <givm/executor.hpp>

#include "../test_source_library.hpp"

namespace givm_test::definition::command_checks
{
namespace
{
    struct invalid_commands_source
    {
        using definition_category = givm::support_view;
        struct definition_type { givm::program_entry entry; };
        bool* finished;

        constexpr std::string_view name() const noexcept { return "Invalid command source"; }

        definition_type compile(givm::definition_compile_context& context) const
        {
            context.add_program(std::tuple{ givm::set_energy{} });
            const std::array<std::size_t, 4> positions{ 0, 0, 2, 2 };
            const std::array damages{
                givm::fixed_damage{ .value = 1, .multiplier_denominator = 0, .type = givm::damage_type::physical },
                givm::fixed_damage{ .value = 2, .multiplier_denominator = 0, .type = static_cast<givm::damage_type>(255) }
            };
            const std::vector<givm::any_command> commands{
                givm::draw_cards{ .positions = positions },
                givm::deal_damage{ .damages = damages },
                givm::set_energy{ .target = { .selection = givm::character_selection::others }, .value = 1 }
            };
            const auto entry = context.add_program(commands);
            context.add_program(std::tuple{ givm::end_game{ static_cast<givm::game_result>(255) } });
            *finished = true;
            return { entry };
        }
    };
}

TEST_CASE("root programs report dynamic input commands in every build mode", "[definition][compile][command_check]")
{
    auto sources = givm_test::make_source_library();
    const auto initialization = std::tuple{
        givm::heal{},
        givm::set_energy{ .target = { static_cast<givm::relative_player>(255),
            std::numeric_limits<std::int32_t>::max(), givm::character_selection::all } }
    };
    const auto round = std::tuple{ givm::add_dice{} };
    const auto result = givm::compile(sources, givm_test::basic_sources, initialization, round, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 3);
    const auto& errors = result.error();
    const auto* healing = std::get_if<givm::heal::error_type>(&errors[0].reason);
    const auto* energy = std::get_if<givm::set_energy::error_type>(&errors[1].reason);
    const auto* dice = std::get_if<givm::add_dice::error_type>(&errors[2].reason);
    REQUIRE(healing);
    REQUIRE(energy);
    REQUIRE(dice);
    CHECK(healing->cause == givm::heal::error_type::reason::dynamic_input_in_root);
    CHECK(energy->cause == givm::set_energy::error_type::reason::dynamic_input_in_root);
    CHECK(dice->cause == givm::add_dice::error_type::reason::dynamic_input_in_root);
    CHECK(errors[0].location.program == givm::program_kind::initialization);
    CHECK(errors[0].location.command_index == 0);
    CHECK(errors[1].location.program == givm::program_kind::initialization);
    CHECK(errors[1].location.command_index == 1);
    CHECK(errors[2].location.program == givm::program_kind::round);
    CHECK(errors[2].location.command_index == 0);
    for(const auto& error : errors)
    {
        CHECK(error.location.stage == givm::compile_stage::program);
        CHECK_FALSE(error.location.source);
    }
}

TEST_CASE("variant command programs collect independent parameter errors with their locations", "[definition][compile][command_check]")
{
    bool finished = false;
    const invalid_commands_source source{ &finished };
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(source));
    const auto result = givm::compile(sources, givm_test::basic_sources, std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 7);
    CHECK(finished);
    const auto& errors = result.error();
    for(std::size_t index = 0; index < errors.size(); ++index)
    {
        const auto& location = errors[index].location;
        CHECK(location.stage == givm::compile_stage::program);
        REQUIRE(location.source);
        CHECK(location.source->name == "Invalid command source");
        CHECK(location.source->category_index == givm::definition_types::index_of<givm::support_view>());
        CHECK(location.program == givm::program_kind::response);
        CHECK(location.program_index == (index < 6 ? 1 : 2));
    }
    for(std::size_t index = 0; index < 2; ++index)
    {
        const auto* error = std::get_if<givm::draw_cards::error_type>(&errors[index].reason);
        REQUIRE(error);
        CHECK(error->cause == givm::draw_cards::error_type::reason::duplicate_position);
        CHECK(error->value == 2 * index);
        CHECK(error->index == 2 * index + 1);
        CHECK(error->first_index == 2 * index);
        CHECK(errors[index].location.command_index == 0);
    }
    for(std::size_t index = 0; index < 3; ++index)
    {
        const auto* error = std::get_if<givm::deal_damage::error_type>(&errors[index + 2].reason);
        REQUIRE(error);
        CHECK(error->index == (index == 0 ? 0 : 1));
        CHECK(error->cause == (index < 2
            ? givm::deal_damage::error_type::reason::zero_multiplier_denominator
            : givm::deal_damage::error_type::reason::invalid_damage_type));
        CHECK(errors[index + 2].location.command_index == 1);
    }
    const auto* energy = std::get_if<givm::set_energy::error_type>(&errors[5].reason);
    const auto* end = std::get_if<givm::end_game::error_type>(&errors[6].reason);
    REQUIRE(energy);
    REQUIRE(end);
    CHECK(energy->cause == givm::set_energy::error_type::reason::invalid_target_selection);
    CHECK(errors[5].location.command_index == 2);
    CHECK(end->cause == givm::end_game::error_type::reason::invalid_result);
    CHECK(end->value == 255);
    CHECK(errors[6].location.command_index == 0);

    const auto drawing_message = givm::error_string(std::get<givm::draw_cards::error_type>(errors[1].reason));
    CHECK(drawing_message.find("positions[3] = 2") != std::string::npos);
    CHECK(drawing_message.find("positions[2]") != std::string::npos);
    const auto damage_message = givm::error_string(std::get<givm::deal_damage::error_type>(errors[4].reason));
    CHECK(damage_message.find("damages[1].type") != std::string::npos);
    CHECK(damage_message.find("255") != std::string::npos);
}

TEST_CASE("fixed commands reject definition IDs outside the selected library", "[definition][compile][command_check]")
{
    const givm_test::reaction_source<givm::card_definition> card{ "Outside card" };
    auto other_sources = givm_test::make_source_library();
    REQUIRE(other_sources.add(card));
    const auto compiled = givm::compile(other_sources, givm_test::basic_sources,
        std::tuple{}, std::tuple{}, givm::compile_mode::normal);
    REQUIRE(compiled);
    const auto card_id = compiled->id_map.get_id<givm::card_definition>(card.name());
    auto sources = givm_test::make_source_library();
    const auto result = givm::compile(sources, givm_test::basic_sources,
        std::tuple{ givm::insert_deck_card{ givm::player_id{ 0 }, card_id } },
        std::tuple{}, givm::compile_mode::normal);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().size() == 1);
    const auto* error = std::get_if<givm::insert_deck_card::error_type>(&result.error()[0].reason);
    REQUIRE(error);
    CHECK(error->cause == givm::insert_deck_card::error_type::reason::invalid_definition);
    CHECK(error->value == card_id.value());
    CHECK(error->limit == 0);
    const auto message = givm::error_string(*error);
    CHECK(message.find("definition ID 0") != std::string::npos);
    CHECK(message.find("[0, 0)") != std::string::npos);
}

TEST_CASE("fixed relative root commands retain their externally supplied player contract", "[definition][compile][command_check]")
{
    auto sources = givm_test::make_source_library();
    const std::vector<givm::any_command> initialization{
        givm::shuffle_deck{ givm::player_id{ 0 } },
        givm::set_energy{ .target = { givm::relative_player::self }, .value = 1 },
        givm::modify_energy{ .target = { givm::relative_player::opponent, 0, givm::character_selection::all }, .delta = -1 },
        givm::end_game{ givm::game_result::both_loss }
    };
    const auto result = givm::compile(sources, givm_test::basic_sources, initialization, std::tuple{}, givm::compile_mode::normal);
    REQUIRE(result);
}
}
