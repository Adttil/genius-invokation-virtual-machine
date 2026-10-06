#ifndef GIVM_TEST_CHARACTER_SOURCE_HPP
#define GIVM_TEST_CHARACTER_SOURCE_HPP

#include <cstdint>
#include <string_view>
#include <array>
#include <span>
#include <givm/givm.hpp>

namespace givm::test
{
    struct initialized_character_source
    {
        using definition_category = givm::character_view;
        struct definition_type { givm::character_state initial_state; };

        std::string_view source_name = "Character";
        givm::character_state initial_state{ .max_health = 10, .max_energy = 3, .health = 10 };

        std::string_view name() const noexcept { return source_name; }
        definition_type compile(givm::definition_compile_context&) const { return { initial_state }; }
        static givm::character_state query(const definition_type& data, const givm::character_initial_state&)
        {
            return data.initial_state;
        }
    };

    // Compile fixture setup through the same context as ordinary definition programs.
    template<class TProgram>
    struct initialization_skill_source
    {
        using definition_category = skill_view;
        struct definition_type { normal_effect entry; };

        TProgram program;
        std::span<const std::string_view> cards{};
        std::span<const std::string_view> supports{};
        std::span<const std::string_view> attachments{};
        std::span<const std::string_view> combat_statuses{};

        std::string_view name() const { return "TestInitialization"; }
        auto card_dependencies() const { return cards; }
        auto support_dependencies() const { return supports; }
        auto attachment_dependencies() const { return attachments; }
        auto combat_status_dependencies() const { return combat_statuses; }
        definition_type compile(definition_compile_context& context) const
        {
            return { context.add_normal_effect(program(context)) };
        }
        static normal_effect handle(const definition_type& data,
            battle_started&, handle_context<skill_view>& context, std::uint32_t = 0)
        {
            return context.invoke(data.entry);
        }
    };

    struct initialization_character_source
    {
        using definition_category = character_view;
        struct definition_type
        {
            character_state initial_state;
            definition_id<skill_view> initialization;
        };

        std::string_view source_name = "InitializationCharacter";
        character_state initial_state{ .max_health = 10, .max_energy = 3, .health = 10 };

        std::string_view name() const { return source_name; }
        auto skill_dependencies() const { return std::array{ std::string_view{ "TestInitialization" } }; }
        definition_type compile(definition_compile_context& context) const
        {
            return { initial_state, context.resolve_id<skill_view>("TestInitialization") };
        }
        static character_state query(const definition_type& data, const character_initial_state&)
        {
            return data.initial_state;
        }
        static definition_id<skill_view> query(const definition_type& data, const character_initial_skill& query)
        {
            return query.skill_index == 0 ? data.initialization : definition_id<skill_view>{};
        }
    };
}

#endif
