#ifndef GIVM_UNIT_TESTS_TABLE_TEST_PASSIVE_SKILL_HPP
#define GIVM_UNIT_TESTS_TABLE_TEST_PASSIVE_SKILL_HPP

#include <concepts>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <givm/definition.hpp>

namespace givm::test
{
    // Existing scenario observers keep their character-oriented assertions while
    // their global responses are supplied by a real passive skill prepended to
    // the initial skill list. Original skill queries still receive indices 0..N.
    template<class TSource>
    struct passive_skill_source : TSource
    {
        using definition_category = skill_view;
        using definition_type = decltype(std::declval<const TSource&>().compile(std::declval<definition_compile_context&>()));

        explicit passive_skill_source(const TSource& source) : TSource{ source } {}

        template<class TEvent>
            requires requires(const definition_type& data, const character_view& character, TEvent& event, handle_context& context)
            {
                TSource::handle(data, character, event, context);
            }
        static program_entry handle(const definition_type& data, const skill_view& self, TEvent& event, handle_context& context)
        {
            return TSource::handle(data, self.character(), event, context);
        }
    };

    template<class TSource>
    struct character_with_passive_skill : TSource
    {
        using source_definition_type = decltype(std::declval<const TSource&>().compile(std::declval<definition_compile_context&>()));
        struct definition_type
        {
            source_definition_type data;
            definition_id<skill_view> passive;
        };

        passive_skill_source<TSource> passive;

        explicit character_with_passive_skill(const TSource& source) : TSource{ source }, passive{ source } {}

        auto skill_dependencies() const
        {
            std::vector<std::string_view> result;
            if constexpr(requires { TSource::skill_dependencies(); })
                for(const auto name : TSource::skill_dependencies()) result.push_back(name);
            result.push_back(passive.name());
            return result;
        }

        definition_type compile(definition_compile_context& context) const
        {
            return { TSource::compile(context), context.resolve_id<skill_view>(passive.name()) };
        }

        static definition_id<skill_view> query(const definition_type& data, const character_initial_skill& query)
        {
            if(query.skill_index == 0) return data.passive;
            if constexpr(requires { TSource::query(data.data, query); })
                return TSource::query(data.data, character_initial_skill{ query.skill_index - 1 });
            else return {};
        }

        template<class TQuery>
            requires(not std::same_as<TQuery, character_initial_skill>)
                && requires(const source_definition_type& data, const TQuery& query) { TSource::query(data, query); }
        static TQuery::result_t query(const definition_type& data, const TQuery& query)
        {
            return TSource::query(data.data, query);
        }
    };

    template<class TSource>
    auto with_passive_skill(const TSource& source)
    {
        return character_with_passive_skill<TSource>{ source };
    }

    template<class TSource>
    auto test_definition_sources(const TSource& source)
    {
        return std::tie(source);
    }

    template<class TSource>
    auto test_definition_sources(const character_with_passive_skill<TSource>& source)
    {
        return std::tie(source, source.passive);
    }
}

#endif
