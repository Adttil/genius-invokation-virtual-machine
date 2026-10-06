#include <array>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <givm/executor.hpp>

#include "../../test_source_library.hpp"
#include "../test_character_source.hpp"

namespace givm_test::executor_instructions::transfer_attachment
{
namespace
{
    constexpr givm::character_id source_character{ givm::player_id{ 1 }, 1 };
    constexpr givm::character_id target_character{ givm::player_id{ 0 }, 1 };

    enum class removal_response { none, change_and_pause, remove, replace };

    struct transfer_log
    {
        bool dynamic = false;
        bool reset = false;
        bool equipment = true;
        bool occupied = false;
        bool followup = false;
        removal_response response = removal_response::none;
        std::size_t rounds = 0;
        std::size_t state_changes = 0;
        std::size_t reapplications = 0;
        givm::attachment_id source;
        givm::attachment_id displaced;
        givm::attachment_id transferred;
        std::vector<givm::attachment_id> removed;
        std::vector<givm::attachment_id> changed;
        std::vector<givm::attachment_state> states;
    };

    struct attachment_source
    {
        using definition_category = givm::attachment_view;
        struct definition_type { transfer_log* log; };

        transfer_log* log;
        std::string_view source_name;
        std::string_view name() const { return source_name; }
        auto tags() const
        {
            return log->equipment ? std::vector<std::string_view>{ "artifact" } : std::vector<std::string_view>{};
        }
        definition_type compile(givm::definition_compile_context&) const { return { log }; }
        static givm::attachment_state query(const definition_type&, const givm::attachment_state_limit&)
        {
            return { 20, 6 };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::attachment_reapplication&, givm::handle_context<givm::attachment_view>&, std::uint32_t = 0)
        {
            ++data.log->reapplications;
            return {};
        }
        static givm::program_entry handle(const definition_type& data,
            givm::attachment_state_changed& event, givm::handle_context<givm::attachment_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            ++data.log->state_changes;
            if(not data.log->followup) CHECK(self.id() == data.log->transferred);
            data.log->changed.push_back(self.id());
            data.log->states.push_back(self.state());
            CHECK(self.character().id() == target_character);
            CHECK(self.state().count == event.current.count);
            CHECK(self.state().round_usages == event.current.round_usages);
            CHECK(context.table()[self.id()].is_valid());
            return {};
        }
    };

    struct transfer_source
    {
        using definition_category = givm::support_view;
        struct definition_type
        {
            transfer_log* log;
            givm::definition_id<givm::attachment_view> moving;
            givm::definition_id<givm::attachment_view> old;
            givm::definition_id<givm::attachment_view> replacement;
            givm::program_entry prepare;
            givm::program_entry transfer;
            givm::program_entry change;
            givm::program_entry remove;
            givm::program_entry replace;
        };

        transfer_log* log;
        std::string_view name() const { return "TransferController"; }
        auto attachment_dependencies() const
        {
            return std::array<std::string_view, 3>{ "Moving", "Old", "Replacement" };
        }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto moving = context.resolve_id<givm::attachment_view>("Moving");
            const auto replacement = context.resolve_id<givm::attachment_view>("Replacement");
            givm::relative_attachment_target source{
                .character = { .player = givm::relative_player::self, .offset = 1 }, .selector = moving };
            if(log->followup) source.selector = givm::equipment_type::artifact;
            const auto command = log->dynamic ? givm::transfer_attachment{} : givm::transfer_attachment{
                .source = source,
                .target = { .player = givm::relative_player::opponent, .offset = 1 },
                .reset_round_usages = log->reset
            };
            givm::program_entry transfer;
            if(log->followup)
            {
                const givm::relative_attachment_target target{
                    .character = { .player = givm::relative_player::opponent, .offset = 1 },
                    .selector = givm::equipment_type::artifact };
                const auto set = log->dynamic ? givm::set_attachment_state{} : givm::set_attachment_state{ target, { 10, 2 } };
                const auto modify = log->dynamic ? givm::modify_attachment_state{} : givm::modify_attachment_state{ target, -2, -1 };
                const auto remove = log->dynamic ? givm::remove_attachment{} : givm::remove_attachment{ target };
                transfer = context.add_program(std::tuple{ command, givm::settle{}, set, givm::settle{}, givm::add_attachment{}, modify, givm::settle{},
                    givm::replace_cards{ givm::player_id{ 0 } }, remove, givm::set_support_state{} });
            }
            else transfer = context.add_program(std::tuple{ givm::set_support_state{}, command, givm::set_support_state{} });
            return { log, moving, context.resolve_id<givm::attachment_view>("Old"),
                replacement,
                log->occupied ? context.add_program(std::tuple{ givm::add_attachment{}, givm::add_attachment{} })
                    : context.add_program(std::tuple{ givm::add_attachment{} }),
                transfer,
                context.add_program(std::tuple{ givm::set_attachment_state{}, givm::settle{}, givm::replace_cards{ givm::player_id{ 0 } } }),
                context.add_program(std::tuple{ givm::remove_attachment{} }),
                context.add_program(std::tuple{ givm::add_attachment{} }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            if(data.log->rounds++ == 0)
            {
                const givm::add_attachment_input first{ source_character, data.moving, { 3, 1 } };
                if(data.log->occupied)
                    return context.invoke(data.prepare, first, givm::add_attachment_input{
                        target_character, data.log->equipment ? data.old : data.moving, { 5, 2 } });
                return context.invoke(data.prepare, first);
            }
            for(const auto attachment : context.table()[source_character].attachments())
                data.log->source = attachment.id();
            if(data.log->occupied)
                for(const auto attachment : context.table()[target_character].attachments())
                    data.log->displaced = attachment.id();
            const givm::set_support_state_input before{ self.id(), { 1, 0 } };
            const givm::set_support_state_input after{ self.id(), { 2, 0 } };
            if(data.log->followup)
            {
                const givm::equipment_target destination{ target_character, givm::equipment_type::artifact };
                const givm::add_attachment_input replacement{ target_character, data.replacement, { 11, 3 } };
                if(data.log->dynamic)
                    return context.invoke(data.transfer,
                        givm::transfer_attachment_input{ givm::equipment_target{ source_character, givm::equipment_type::artifact },
                            target_character, data.log->reset },
                        givm::set_attachment_state_input{ destination, { 10, 2 } }, replacement,
                        givm::modify_attachment_state_input{ destination, -2, -1 },
                        givm::remove_attachment_input{ destination }, after);
                return context.invoke(data.transfer, replacement, after);
            }
            if(data.log->dynamic)
                return context.invoke(data.transfer, before, givm::transfer_attachment_input{
                    data.log->source, target_character, data.log->reset }, after);
            return context.invoke(data.transfer, before, after);
        }
        static givm::program_entry handle(const definition_type& data,
            givm::attachment_removed& event, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            data.log->removed.push_back(event.attachment);
            CHECK_FALSE(context.table()[event.attachment].is_valid());
            if(event.attachment != data.log->displaced) return {};
            CHECK_FALSE(context.table()[data.log->source].is_valid());
            CHECK_FALSE(context.table()[source_character].has(givm::equipment_type::artifact));
            const auto target = context.table()[target_character];
            REQUIRE(target.has(givm::equipment_type::artifact));
            const auto moved = target.get(givm::equipment_type::artifact);
            data.log->transferred = moved.id();
            CHECK(moved.id() != data.log->source);
            CHECK(moved.definition_id() == data.moving);
            CHECK(moved.state().count == 3);
            CHECK(moved.state().round_usages == (data.log->reset ? 6u : 1u));
            CHECK(data.log->state_changes == 0);
            CHECK(data.log->reapplications == 0);
            switch(data.log->response)
            {
                case removal_response::change_and_pause:
                    return context.invoke(data.change, givm::set_attachment_state_input{ moved.id(), { 9, 2 } });
                case removal_response::remove:
                    return context.invoke(data.remove, givm::remove_attachment_input{ moved.id() });
                case removal_response::replace:
                    return context.invoke(data.replace, givm::add_attachment_input{ target_character, data.replacement, { 11, 4 } });
                default:
                    return {};
            }
        }
    };

    auto make_library(givm::compile_mode mode, transfer_log& log)
    {
        auto sources = givm_test::make_source_library();
        const transfer_source controller{ &log };
        const attachment_source moving{ &log, "Moving" };
        const attachment_source old{ &log, "Old" };
        const attachment_source replacement{ &log, "Replacement" };
        const givm::test::initialized_character_source character;
        REQUIRE(sources.add(controller, moving, old, replacement, character));
        const std::array support_names{ controller.name() };
        const givm::test::initialization_skill_source initialization{
            [](givm::definition_compile_context& context)
            {
                return std::tuple{ givm::add_support{
                    .definition = context.resolve_id<givm::support_view>("TransferController"), .state = {} } };
            }, {}, support_names };
        const givm::test::initialization_character_source driver;
        REQUIRE(sources.add(initialization, driver));
        return givm_test::require_success(compile(sources, givm_test::basic_sources, std::tuple{
            givm::start_battle{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss }
        }, std::tuple{}, mode));
    }

    givm::table make_table(const givm::definition_library& library, const givm::issued_id_map& ids)
    {
        givm::table table{ { .round_number = 1, .active_player = givm::player_id{ 0 }, .self_player = givm::player_id{ 1 } },
            { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } },
            { .active_character = givm::character_id{ givm::player_id{ 1 }, 0 } } };
        const auto character = ids.get_id<givm::character_view>("Character");
        load_deck(table, library, { .characters = { character, character } },
            { .characters = { ids.get_id<givm::character_view>("InitializationCharacter"), character } });
        return table;
    }

    givm::execution_state advance(givm_test::executor_driver& executor, const givm::definition_library& library, givm::table& table)
    {
        auto random = [] { return std::uint32_t{ 0 }; };
        for(std::size_t step = 0; step < 20; ++step)
        {
            const auto state = executor.advance(library, table, random);
            if(state == givm::execution_state::finished || state == givm::execution_state::card_selection) return state;
        }
        FAIL("attachment transfer did not finish");
        return givm::execution_state::finished;
    }

    struct zero_health_source
    {
        using definition_category = givm::support_view;
        struct definition_type
        {
            std::vector<givm::attachment_id>* removed;
            givm::definition_id<givm::attachment_view> attachment;
            givm::program_entry entry;
        };
        std::vector<givm::attachment_id>* removed;
        std::string_view name() const { return "ZeroHealthController"; }
        auto attachment_dependencies() const { return std::array<std::string_view, 1>{ "RetainedAttachment" }; }
        definition_type compile(givm::definition_compile_context& context) const
        {
            const auto attachment = context.resolve_id<givm::attachment_view>("RetainedAttachment");
            const givm::relative_attachment_target target{ .selector = attachment };
            return { removed, attachment, context.add_program(std::tuple{
                givm::add_attachment{}, givm::add_attachment{}, givm::set_attachment_state{ target, { 10, 3 } },
                givm::modify_attachment_state{ target, -1, -1 }, givm::remove_attachment{ target }
            }) };
        }
        static givm::program_entry handle(const definition_type& data,
            givm::round_started&, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            const auto self = context.entity();
            return context.invoke(data.entry,
                givm::add_attachment_input{ { self.player().id(), 0 }, data.attachment, { 3, 1 } },
                givm::add_attachment_input{ { self.player().id(), 1 }, data.attachment, { 5, 4 } });
        }
        static givm::program_entry handle(const definition_type& data,
            givm::attachment_removed& event, givm::handle_context<givm::support_view>& context, std::uint32_t = 0)
        {
            data.removed->push_back(event.attachment);
            const auto attachment = context.table()[event.attachment];
            CHECK(attachment.character().id().index == 0);
            CHECK(attachment.character().state().health == 0);
            CHECK(attachment.state().count == 9);
            CHECK(attachment.state().round_usages == 2);
            return {};
        }
    };
}

TEST_CASE("attachment transfer preserves state or resets only round usages without its own notifications", "[transfer_attachment]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    transfer_log log;
    log.dynamic = GENERATE(false, true);
    log.reset = GENERATE(false, true);
    log.equipment = GENERATE(false, true);
    log.occupied = GENERATE(false, true);
    const auto [library, ids] = make_library(mode, log);
    auto table = make_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK_FALSE(table[log.source].is_valid());
    CHECK(table[log.source].state().count == 3);
    CHECK(table[log.source].state().round_usages == 1);
    CHECK(log.state_changes == 0);
    CHECK(log.reapplications == 0);
    CHECK(log.removed == (log.equipment && log.occupied ? std::vector{ log.displaced } : std::vector<givm::attachment_id>{}));
    CHECK(std::ranges::distance(table[source_character].attachments()) == 0);
    std::size_t transferred = 0;
    for(const auto attachment : table[target_character].attachments())
    {
        if(log.occupied && not log.equipment && attachment.id() == log.displaced)
        {
            CHECK(attachment.state().count == 5);
            CHECK(attachment.state().round_usages == 2);
            continue;
        }
        ++transferred;
        CHECK(attachment.id() != log.source);
        CHECK(attachment.definition_id() == ids.get_id<givm::attachment_view>("Moving"));
        CHECK(attachment.state().count == 3);
        CHECK(attachment.state().round_usages == (log.reset ? 6u : 1u));
    }
    CHECK(transferred == 1);
    if(log.equipment)
    {
        CHECK_FALSE(table[source_character].has(givm::equipment_type::artifact));
        REQUIRE(table[target_character].has(givm::equipment_type::artifact));
        CHECK(table[target_character].get(givm::equipment_type::artifact).definition_id() == ids.get_id<givm::attachment_view>("Moving"));
    }
    for(const auto support : table[givm::player_id{ 1 }].supports()) CHECK(support.state().count == 2);
}

TEST_CASE("displaced equipment observes completed transfer and can change it across a copied pause", "[transfer_attachment]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    transfer_log log;
    log.dynamic = GENERATE(false, true);
    log.reset = true;
    log.occupied = true;
    log.response = removal_response::change_and_pause;
    const auto [library, ids] = make_library(mode, log);
    auto table = make_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
    CHECK(table[log.transferred].state().count == 9);
    CHECK(table[log.transferred].state().round_usages == 2);
    CHECK(log.state_changes == 1);
    CHECK(log.removed == std::vector{ log.displaced });
    auto copied_table = table;
    auto copied_executor = executor;
    const auto resume = [&](givm_test::executor_driver& execution, givm::table& branch)
    {
        execution.submitted(execution.view_in<givm::execution_state::card_selection>().select(library, branch, givm_test::zero_random, {}));
        REQUIRE(advance(execution, library, branch) == givm::execution_state::finished);
        REQUIRE(branch[log.transferred].is_valid());
        CHECK(branch[log.transferred].state().count == 9);
        CHECK(branch[log.transferred].state().round_usages == 2);
        CHECK(log.state_changes == 1);
        CHECK(log.removed == std::vector{ log.displaced });
        for(const auto support : branch[givm::player_id{ 1 }].supports()) CHECK(support.state().count == 2);
    };
    resume(executor, table);
    resume(copied_executor, copied_table);
}

TEST_CASE("displaced equipment may remove or replace the transferred equipment permanently", "[transfer_attachment]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    transfer_log log;
    log.dynamic = GENERATE(false, true);
    log.reset = true;
    log.occupied = true;
    log.response = GENERATE(removal_response::remove, removal_response::replace);
    const auto [library, ids] = make_library(mode, log);
    auto table = make_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    CHECK_FALSE(table[log.source].is_valid());
    CHECK_FALSE(table[log.transferred].is_valid());
    CHECK(log.removed == std::vector{ log.displaced, log.transferred });
    CHECK(log.state_changes == 0);
    CHECK(log.reapplications == 0);
    const auto target = table[target_character];
    CHECK(target.has(givm::equipment_type::artifact) == (log.response == removal_response::replace));
    if(log.response == removal_response::replace)
    {
        const auto replacement = target.get(givm::equipment_type::artifact);
        CHECK(replacement.definition_id() == ids.get_id<givm::attachment_view>("Replacement"));
        CHECK(replacement.state().count == 11);
        CHECK(replacement.state().round_usages == 4);
    }
    for(const auto support : table[givm::player_id{ 1 }].supports()) CHECK(support.state().count == 2);
}

TEST_CASE("equipment targets resolve the current slot when each command executes", "[transfer_attachment][attachment_target]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    transfer_log log;
    log.dynamic = GENERATE(false, true);
    log.reset = true;
    log.occupied = true;
    log.followup = true;
    const auto [library, ids] = make_library(mode, log);
    auto table = make_table(library, ids);
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::card_selection);
    CHECK_FALSE(table[log.source].is_valid());
    CHECK_FALSE(table[log.transferred].is_valid());
    REQUIRE(log.changed.size() == 2);
    REQUIRE(log.states.size() == 2);
    CHECK(log.changed[0] == log.transferred);
    CHECK(log.states[0].count == 10);
    CHECK(log.states[0].round_usages == 2);
    const auto replacement = table[target_character].get(givm::equipment_type::artifact);
    CHECK(replacement.id() == log.changed[1]);
    CHECK(replacement.id() != log.transferred);
    CHECK(replacement.definition_id() == ids.get_id<givm::attachment_view>("Replacement"));
    CHECK(log.states[1].count == 9);
    CHECK(log.states[1].round_usages == 2);
    CHECK(replacement.state().count == 9);
    CHECK(replacement.state().round_usages == 2);
    CHECK(log.removed == std::vector{ log.displaced, log.transferred });
    const auto replacement_id = replacement.id();
    const auto paused_log = log;
    auto copied_table = table;
    auto copied_executor = executor;
    const auto resume = [&](givm_test::executor_driver& execution, givm::table& branch)
    {
        log = paused_log;
        execution.submitted(execution.view_in<givm::execution_state::card_selection>().select(library, branch, givm_test::zero_random, {}));
        REQUIRE(advance(execution, library, branch) == givm::execution_state::finished);
        CHECK_FALSE(branch[target_character].has(givm::equipment_type::artifact));
        CHECK_FALSE(branch[replacement_id].is_valid());
        CHECK(log.removed == std::vector{ log.displaced, log.transferred, replacement_id });
        CHECK(log.state_changes == 2);
        CHECK(log.reapplications == 0);
        for(const auto support : branch[givm::player_id{ 1 }].supports()) CHECK(support.state().count == 2);
    };
    resume(executor, table);
    resume(copied_executor, copied_table);
}

TEST_CASE("fixed attachment state and removal commands retain a zero-health character as their target", "[attachment_target]")
{
    const auto mode = GENERATE(givm::compile_mode::normal, givm::compile_mode::observed);
    std::vector<givm::attachment_id> removed;
    const zero_health_source controller{ &removed };
    const givm_test::reaction_source<givm::attachment_view> attachment{ "RetainedAttachment" };
    const givm::test::initialized_character_source zero_health{ "ZeroHealth", { .max_health = 10, .health = 0 } };
    const givm::test::initialized_character_source living;
    auto sources = givm_test::make_source_library();
    REQUIRE(sources.add(controller, attachment, zero_health, living));
    const std::array support_names{ controller.name() };
    const givm::test::initialization_skill_source initialization{
        [](givm::definition_compile_context& context)
        {
            return std::tuple{ givm::add_support{
                .definition = context.resolve_id<givm::support_view>("ZeroHealthController") } };
        }, {}, support_names };
    const givm::test::initialization_character_source driver;
    REQUIRE(sources.add(initialization, driver));
    const auto [library, ids] = givm_test::require_success(compile(sources, givm_test::basic_sources, std::tuple{
        givm::start_battle{}, givm::settle{}, givm::start_round{}, givm::settle{}, givm::end_game{ givm::game_result::both_loss }
    }, std::tuple{}, mode));
    givm::table table{ { .round_number = 1, .self_player = givm::player_id{ 0 } },
        { .active_character = givm::character_id{ givm::player_id{ 0 }, 0 } }, {} };
    load_deck(table, library, { .characters = { ids.get_id<givm::character_view>(zero_health.name()),
        ids.get_id<givm::character_view>(driver.name()) } }, {});
    givm_test::executor_driver executor;
    executor.start(library, table);
    REQUIRE(advance(executor, library, table) == givm::execution_state::finished);
    REQUIRE(removed.size() == 1);
    CHECK(table[givm::character_id{ givm::player_id{ 0 }, 0 }].attachments().empty());
    auto remaining = table[givm::character_id{ givm::player_id{ 0 }, 1 }].attachments();
    REQUIRE(std::ranges::distance(remaining) == 1);
    CHECK((*remaining.begin()).state().count == 5);
    CHECK((*remaining.begin()).state().round_usages == 4);
}
}
