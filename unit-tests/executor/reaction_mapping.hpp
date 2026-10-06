#include <array>
#include <string_view>
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <givm/givm.hpp>

#include "../test_source_library.hpp"

namespace givm_test::executor::reaction_mapping
{
namespace
{
    struct reaction_log
    {
        std::vector<givm::player_id> origins;
        std::vector<givm::reaction_id> reactions;
    };

    struct reaction_source
    {
        using definition_category = givm::reaction_view;
        struct definition_type { std::uint32_t bonus; reaction_log* log; givm::program_entry effect; };
        std::string_view source_name;
        std::uint32_t bonus;
        reaction_log* log = nullptr;
        std::string_view name() const { return source_name; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { bonus, log, context.add_program(givm::modify_energy{}) };
        }
        static givm::element_aura query(const definition_type&, const givm::reaction_aura&)
        {
            return givm::element_aura::dendro;
        }
        static givm::program_entry handle(const definition_type& data, givm::damage_calculation& event,
            givm::handle_context<givm::reaction_view>&, std::uint32_t = 0)
        {
            event.value += data.bonus;
            return {};
        }
        static givm::program_entry handle(const definition_type& data, givm::elemental_reaction_will_occur& event,
            givm::handle_context<givm::reaction_view>& context, std::uint32_t = 0)
        {
            if(not data.log) return {};
            data.log->origins.push_back(event.source_player());
            data.log->reactions.push_back(context.entity().id());
            const std::array targets{ givm::character_id{ event.source_player(), 0 } };
            return context.invoke(data.effect, givm::modify_energy_input{ targets, 1 });
        }
    };

    struct character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::character_state initial; givm::definition_id<givm::reaction_view> replacement; };
        std::string_view source_name;
        std::string_view reaction_name;
        givm::character_state initial{ .max_health = 20, .max_energy = 3, .health = 20 };
        std::string_view name() const { return source_name; }
        auto reaction_dependencies() const { return std::array{ reaction_name }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            return { initial, context.resolve_id<givm::reaction_view>(reaction_name) };
        }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return data.initial;
        }
        static givm::definition_id<givm::reaction_view> query(const definition_type& data,
            const givm::character_reaction_override& query)
        {
            return query.slot == givm::elemental_reaction::electro_charged ? data.replacement
                : givm::definition_id<givm::reaction_view>{};
        }
    };
}

TEST_CASE("reaction maps persist after character death and distinguish origin from mapping owner", "[reactions][settlement]")
{
    using namespace givm;
    reaction_log log;
    const reaction_source ordinary{ "Ordinary", 1 };
    const reaction_source lunar{ "Lunar", 4, &log };
    const character_source own{ "Own", ordinary.name(),
        { .max_health = 20, .max_energy = 3, .health = 20, .aura = element_aura::hydro } };
    const character_source fallen{ "Fallen", lunar.name(), { .max_health = 20, .health = 0, .alive = false } };
    definition_source_library sources;
    REQUIRE(sources.add(ordinary, lunar, own, fallen));
    reaction_definition_names names{ ordinary.name() };
    const auto [library, ids] = require_success(compile(sources, names, std::tuple{
        deal_damage{ .target = { relative_player::self, 0 }, .value = 1, .type = damage_type::electro },
        settle{}, end_game{ game_result::both_loss } }, std::tuple{}, compile_mode::normal));
    const character_id target{ player_id{ 0 }, 0 };
    givm::table card_table{ { .self_player = player_id{ 0 } }, { .active_character = target }, {} };
    load_deck(card_table, library,
        { .characters = { ids.get_id<character_view>(own.name()) } },
        { .characters = { ids.get_id<character_view>(fallen.name()) } });
    CHECK(card_table[reaction_id{ player_id{ 1 }, elemental_reaction::electro_charged }].definition_id()
        == ids.get_id<reaction_view>(lunar.name()));
    givm::executor execution;
    REQUIRE(execution.start(library, card_table).resume(library, card_table, zero_random) == execution_state::finished);
    CHECK(card_table[target].state().health == 15);
    CHECK(card_table[target].state().aura == element_aura::dendro);
    CHECK(card_table[target].state().energy == 1);
    CHECK(log.origins == std::vector{ player_id{ 0 } });
    CHECK(log.reactions == std::vector{ reaction_id{ player_id{ 1 }, elemental_reaction::electro_charged } });
}

TEST_CASE("deck reaction overrides follow character order and later definitions replace earlier ones", "[reactions][deck]")
{
    using namespace givm;
    const reaction_source ordinary{ "Ordinary", 1 };
    const reaction_source lunar{ "Lunar", 4 };
    const character_source first{ "First", ordinary.name() };
    const character_source second{ "Second", lunar.name() };
    definition_source_library sources;
    REQUIRE(sources.add(ordinary, lunar, first, second));
    reaction_definition_names names{ ordinary.name() };
    const auto [library, ids] = require_success(compile(sources, names, std::tuple{}, std::tuple{}, compile_mode::normal));
    const auto a = ids.get_id<character_view>(first.name());
    const auto b = ids.get_id<character_view>(second.name());
    givm::table ordered;
    load_deck(ordered, library, { .characters = { b, b, a } }, { .characters = { a, a, b } });
    CHECK(ordered[reaction_id{ player_id{ 0 }, elemental_reaction::electro_charged }].definition_id()
        == ids.get_id<reaction_view>(ordinary.name()));
    CHECK(ordered[reaction_id{ player_id{ 1 }, elemental_reaction::electro_charged }].definition_id()
        == ids.get_id<reaction_view>(lunar.name()));
    givm::table compatible;
    load_deck(compatible, library, { .characters = { b, b } }, { .characters = { a } });
    CHECK(compatible[reaction_id{ player_id{ 0 }, elemental_reaction::electro_charged }].definition_id()
        == ids.get_id<reaction_view>(lunar.name()));
}
}
