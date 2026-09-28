#ifndef GIVM_DEFINITION_SOURCE_VIEW_HPP
#define GIVM_DEFINITION_SOURCE_VIEW_HPP

#include <any>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "program_entry.hpp"
#include "subscribed_events.hpp"
#include "supported_queries.hpp"
#include "definition_categories.hpp"
#include "history_summary.hpp"

namespace givm
{
    class definition_compile_context;
    class definition_library;
    class handle_context;

    using definition_data = std::any;

    template<class TEntity, class TEvent>
    using handle_fn_t = program_entry (*)(
        const definition_data&,
        const TEntity&,
        TEvent&,
        handle_context&
    );
}

namespace givm::detail
{
    template<class TEvent>
    using history_handle_fn_t = void (*)(const definition_data&, history_summary_state,
        const TEvent&, const table&, const definition_library&);

    template<class... TEvents>
    using history_handle_getters = std::tuple<history_handle_fn_t<TEvents>(*)(const void*)...>;

    struct history_source_functions
    {
        history_summary_layout(*layout)(const void*, const definition_compile_context&);
        subscribed_events<history_summary_definition>::apply<history_handle_getters> handles;
    };

    template<class TQuery>
    using query_fn_t = TQuery::result_t (*)(const definition_data&, const TQuery&);

    template<class TQuery>
    inline TQuery::result_t default_query(const definition_data&, const TQuery& query)
    {
        static_assert(std::same_as<decltype(query_default(query)), typename TQuery::result_t>);
        return query_default(query);
    }

    using definition_dependency_lists =
        std::array<std::vector<std::string_view>, definition_types::size()>;

    struct definition_source_declarations
    {
        std::vector<std::string_view> tags;
        definition_dependency_lists dependencies;
    };

    template<class TItems>
    std::vector<std::string_view> collect_definition_strings(TItems&& items)
    {
        std::vector<std::string_view> result;
        for(auto&& item : items)
        {
            result.emplace_back(std::string_view{ item });
        }
        return result;
    }

    template<class TSource>
    constexpr decltype(auto) definition_tags(const TSource& source)
    {
        if constexpr(requires { source.tags(); })
        {
            return source.tags();
        }
        else
        {
            return std::array<std::string_view, 0>{};
        }
    }

    template<class TCategory, class TSource>
    constexpr decltype(auto) definition_dependencies(const TSource& source)
    {
        if constexpr(std::same_as<TCategory, card_definition>)
        {
            if constexpr(requires { source.card_dependencies(); }) return source.card_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, status_definition>)
        {
            if constexpr(requires { source.status_dependencies(); }) return source.status_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, support_view>)
        {
            if constexpr(requires { source.support_dependencies(); }) return source.support_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, summon_view>)
        {
            if constexpr(requires { source.summon_dependencies(); }) return source.summon_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, combat_status_view>)
        {
            if constexpr(requires { source.combat_status_dependencies(); })
                return source.combat_status_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, character_view>)
        {
            if constexpr(requires { source.character_dependencies(); }) return source.character_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, skill_view>)
        {
            if constexpr(requires { source.skill_dependencies(); }) return source.skill_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else if constexpr(std::same_as<TCategory, history_summary_definition>)
        {
            if constexpr(requires { source.history_summary_dependencies(); })
                return source.history_summary_dependencies();
            else return std::array<std::string_view, 0>{};
        }
        else
        {
            static_assert(std::same_as<TCategory, attachment_view>);
            if constexpr(requires { source.attachment_dependencies(); })
                return source.attachment_dependencies();
            else return std::array<std::string_view, 0>{};
        }
    }

    template<class TSource>
    using definition_for_source_t = std::remove_cvref_t<decltype(
        std::declval<const TSource&>().compile(std::declval<definition_compile_context&>())
    )>;

    template<class TSource>
    inline constexpr bool is_dynamic_source = []
    {
        if constexpr(requires { TSource::is_dynamic; })
        {
            static_assert(std::same_as<std::remove_cv_t<decltype(TSource::is_dynamic)>, bool>);
            return TSource::is_dynamic;
        }
        else
        {
            return false;
        }
    }();
}

namespace givm
{
    template<class TCategory>
    class definition_source_view
    {
    public:
        template<class TSource>
            requires std::same_as<typename TSource::definition_category, TCategory>
        constexpr definition_source_view(const TSource& source)
        : source_{ &source }, rtti_{ &rtti_for<TSource> }
        {}

        std::string_view name() const
        {
            return rtti_->name(source_);
        }

        std::vector<std::string_view> tags() const
        {
            return rtti_->tags(source_);
        }

        template<class TDependencyCategory>
        std::vector<std::string_view> dependencies() const
        {
            return rtti_->dependencies[definition_types::index_of<TDependencyCategory>()](source_);
        }

    private:
        template<class TView, class TEvent>
        using handle_fn_getter_t = handle_fn_t<TView, TEvent> (*)(const void*);

        template<class TView, class TSequence>
        struct handle_fn_getter_tuple_for_view_impl;

        template<class TView, std::size_t... I>
        struct handle_fn_getter_tuple_for_view_impl<TView, std::index_sequence<I...>>
        {
            using type = std::tuple<
                handle_fn_getter_t<TView, typename subscribed_events<TView>::template type_at<I>>...
            >;
        };

        template<class TView>
        using handle_fn_getter_tuple_for_view_t = typename handle_fn_getter_tuple_for_view_impl<
            TView,
            std::make_index_sequence<subscribed_events<TView>::size()>
        >::type;

        template<class TSequence>
        struct handle_fn_getter_tuple_impl;

        template<std::size_t... I>
        struct handle_fn_getter_tuple_impl<std::index_sequence<I...>>
        {
            using type = std::tuple<
                handle_fn_getter_tuple_for_view_t<typename views_of_definition<TCategory>::template type_at<I>>...
            >;
        };

        using handle_fn_getter_tuple_t = typename handle_fn_getter_tuple_impl<
            std::make_index_sequence<views_of_definition<TCategory>::size()>
        >::type;

        template<class TQuery>
        using query_fn_getter_t = detail::query_fn_t<TQuery> (*)(const void*);

        template<class... TQueries>
        using query_fn_getter_tuple_for = std::tuple<query_fn_getter_t<TQueries>...>;

        using query_fn_getter_tuple_t = supported_queries<TCategory>::template apply<query_fn_getter_tuple_for>;

        struct rtti_t
        {
            std::string_view(*name)(const void*);
            std::vector<std::string_view>(*tags)(const void*);
            std::array<std::vector<std::string_view>(*)(const void*), definition_types::size()> dependencies;
            definition_data(*compile)(const void*, definition_compile_context&);
            handle_fn_getter_tuple_t handle_fn_getters;
#ifdef _MSC_VER
            [[msvc::no_unique_address]]
#else
            [[no_unique_address]]
#endif
            query_fn_getter_tuple_t query_fn_getters;
#ifdef _MSC_VER
            [[msvc::no_unique_address]]
#else
            [[no_unique_address]]
#endif
            std::conditional_t<std::same_as<TCategory, history_summary_definition>,
                detail::history_source_functions, std::tuple<>> history;
        };

        definition_data compile(definition_compile_context& context) const
        {
            return rtti_->compile(source_, context);
        }

        detail::definition_source_declarations declarations() const
        {
            detail::definition_source_declarations result{
                .tags = tags()
            };
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                ((result.dependencies[I] =
                    rtti_->dependencies[I](source_)), ...);
            }(std::make_index_sequence<definition_types::size()>{});
            return result;
        }

        template<class TView, class TEvent>
        handle_fn_t<TView, TEvent> get_handle_fn() const
        {
            const auto getter = std::get<subscribed_events<TView>::template index_of<TEvent>()>(
                std::get<views_of_definition<TCategory>::template index_of<TView>()>(
                    rtti_->handle_fn_getters
                )
            );
            return getter(source_);
        }

        template<class TSource>
        static constexpr rtti_t make_rtti()
        {
            return {
                .name = +[](const void* source)
                {
                    return static_cast<const TSource*>(source)->name();
                },
                .tags = +[](const void* source)
                {
                    return detail::collect_definition_strings(
                        detail::definition_tags(*static_cast<const TSource*>(source))
                    );
                },
                .dependencies = make_dependencies<TSource>(
                    std::make_index_sequence<definition_types::size()>{}
                ),
                .compile = +[](const void* source, definition_compile_context& context)
                {
                    using result_type = decltype(static_cast<const TSource*>(source)->compile(context));
                    static_assert(not std::is_reference_v<result_type>);
                    static_assert(not std::is_void_v<result_type>);
                    return definition_data{ static_cast<const TSource*>(source)->compile(context) };
                },
                .handle_fn_getters = make_handle_fn_getters<TSource>(
                    std::make_index_sequence<views_of_definition<TCategory>::size()>{}
                ),
                .query_fn_getters = make_query_fn_getters<TSource>(
                    std::make_index_sequence<supported_queries<TCategory>::size()>{}
                ),
                .history = make_history_functions<TSource>()
            };
        }

        template<class TSource, class TEvent>
        static detail::history_handle_fn_t<TEvent> make_history_handle(const TSource& source)
        {
            if constexpr(detail::is_dynamic_source<TSource>)
            {
                static_assert(std::same_as<decltype(source.template can_handle<TEvent>()), bool>);
                if(not source.template can_handle<TEvent>()) return nullptr;
            }
            using definition_type = detail::definition_for_source_t<TSource>;
            if constexpr(requires(const definition_type& definition, history_summary_state state,
                const TEvent& event, const table& card_table, const definition_library& library)
            {
                TSource::handle(definition, state, event, card_table, library);
            })
            {
                static_assert(std::same_as<decltype(TSource::handle(std::declval<const definition_type&>(),
                    std::declval<history_summary_state>(), std::declval<const TEvent&>(),
                    std::declval<const table&>(), std::declval<const definition_library&>())), void>);
                return +[](const definition_data& data, history_summary_state state,
                    const TEvent& event, const table& card_table, const definition_library& library)
                {
                    TSource::handle(std::any_cast<const definition_type&>(data), state, event, card_table, library);
                };
            }
            else if constexpr(detail::is_dynamic_source<TSource>)
                throw std::invalid_argument{ "dynamic source enables an unavailable history handler" };
            else return nullptr;
        }

        template<class TSource>
        static constexpr auto make_history_functions()
        {
            if constexpr(not std::same_as<TCategory, history_summary_definition>) return std::tuple<>{};
            else
            {
                return detail::history_source_functions{
                    .layout = +[](const void* source, const definition_compile_context& context)
                    {
                        return history_summary_layout{ static_cast<const TSource*>(source)->layout(context) };
                    },
                    .handles = []<class... TEvents>(type_list<TEvents...>)
                    {
                        return detail::history_handle_getters<TEvents...>{
                            (+[](const void* source)
                            {
                                return make_history_handle<TSource, TEvents>(*static_cast<const TSource*>(source));
                            })...
                        };
                    }(typename subscribed_events<history_summary_definition>::template apply<type_list>{})
                };
            }
        }

        template<class TSource>
        static constexpr rtti_t rtti_for = make_rtti<TSource>();

        template<class TQuery>
        detail::query_fn_t<TQuery> get_query_fn() const
        {
            const auto getter = std::get<supported_queries<TCategory>::template index_of<TQuery>()>(
                rtti_->query_fn_getters
            );
            return getter(source_);
        }

        template<class TSource, std::size_t... I>
        static constexpr query_fn_getter_tuple_t make_query_fn_getters(std::index_sequence<I...>)
        {
            return {
                (+[](const void* source)
                {
                    using query_type = typename supported_queries<TCategory>::template type_at<I>;
                    return make_query_fn<TSource, query_type>(*static_cast<const TSource*>(source));
                })...
            };
        }

        template<class TSource, class TQuery>
        static detail::query_fn_t<TQuery> make_query_fn(const TSource& source)
        {
            if constexpr(detail::is_dynamic_source<TSource>)
            {
                static_assert(std::same_as<decltype(source.template can_query<TQuery>()), bool>);
                if(not source.template can_query<TQuery>())
                {
                    return nullptr;
                }
            }

            using definition_type = detail::definition_for_source_t<TSource>;
            if constexpr(requires(const definition_type& definition, const TQuery& query)
            {
                TSource::query(definition, query);
            })
            {
                static_assert(std::same_as<decltype(TSource::query(
                    std::declval<const definition_type&>(), std::declval<const TQuery&>()
                )), typename TQuery::result_t>);
                return +[](const definition_data& data, const TQuery& query) -> TQuery::result_t
                {
                    return TSource::query(std::any_cast<const definition_type&>(data), query);
                };
            }
            else if constexpr(detail::is_dynamic_source<TSource>)
            {
                throw std::invalid_argument{ "dynamic source enables an unavailable query" };
            }
            else
            {
                return nullptr;
            }
        }

        template<class TSource, std::size_t... I>
        static constexpr auto make_dependencies(std::index_sequence<I...>)
        {
            return std::array<std::vector<std::string_view>(*)(const void*), definition_types::size()>{
                (+[](const void* source)
                {
                    using dependency_category = typename definition_types::template type_at<I>;
                    return detail::collect_definition_strings(
                        detail::definition_dependencies<dependency_category>(
                            *static_cast<const TSource*>(source)
                        )
                    );
                })...
            };
        }

        template<class TSource, std::size_t... I>
        static constexpr handle_fn_getter_tuple_t make_handle_fn_getters(std::index_sequence<I...>)
        {
            return {
                make_handle_fn_getters_for_view<
                    TSource,
                    typename views_of_definition<TCategory>::template type_at<I>
                >()...
            };
        }

        template<class TSource, class TView, std::size_t... I>
        static constexpr handle_fn_getter_tuple_for_view_t<TView> make_handle_fn_getters_for_view(
            std::index_sequence<I...>
        )
        {
            return {
                make_handle_fn_getter<
                    TSource,
                    TView,
                    typename subscribed_events<TView>::template type_at<I>
                >()...
            };
        }

        template<class TSource, class TView>
        static constexpr handle_fn_getter_tuple_for_view_t<TView> make_handle_fn_getters_for_view()
        {
            return make_handle_fn_getters_for_view<TSource, TView>(
                std::make_index_sequence<subscribed_events<TView>::size()>{}
            );
        }

        template<class TSource, class TView, class TEvent>
        static constexpr handle_fn_getter_t<TView, TEvent> make_handle_fn_getter()
        {
            return +[](const void* source)
            {
                return make_handle_fn<TSource, TView, TEvent>(*static_cast<const TSource*>(source));
            };
        }

        template<class TSource, class TView, class TEvent>
        static handle_fn_t<TView, TEvent> make_handle_fn(const TSource& source)
        {
            if constexpr(detail::is_dynamic_source<TSource>)
            {
                static_assert(std::same_as<decltype(source.template can_handle<TView, TEvent>()), bool>);
                if(not source.template can_handle<TView, TEvent>())
                {
                    return nullptr;
                }
            }

            using definition_type = detail::definition_for_source_t<TSource>;
            if constexpr(requires(
                const definition_type& definition,
                const TView& entity,
                TEvent& event,
                handle_context& context
            )
            {
                TSource::handle(definition, entity, event, context);
            })
            {
                using result_type = decltype(TSource::handle(
                    std::declval<const definition_type&>(),
                    std::declval<const TView&>(),
                    std::declval<TEvent&>(),
                    std::declval<handle_context&>()
                ));
                static_assert(std::same_as<result_type, program_entry>);

                return +[](
                    const definition_data& data,
                    const TView& entity,
                    TEvent& event,
                    handle_context& context
                )
                {
                    return TSource::handle(
                        std::any_cast<const definition_type&>(data),
                        entity,
                        event,
                        context
                    );
                };
            }
            else if constexpr(detail::is_dynamic_source<TSource>)
            {
                throw std::invalid_argument{ "dynamic source enables an unavailable event handler" };
            }
            else
            {
                return nullptr;
            }
        }

        const void* source_;
        const rtti_t* rtti_;

        friend class definition_library;
        friend class definition_source_library;
    };
}

#endif
