#include "../executor_access.hpp"
#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition.hpp>

using namespace givm;

namespace
{
    struct marker_instruction
    {
        using context_type = void;

        int value;

        execution_state execute(const definition_library&, card_table&, detail::execution_context&, random_fn&) const noexcept
        {
            return value != 0 ? detail::continue_execution : execution_state::action;
        }
    };

}

TEST_CASE("initialization and round programs accept tuple-like and range forms", "[definition][library]")
{
    const auto initialization = std::tuple{
        marker_instruction{ 1 },
        marker_instruction{ 4 }
    };
    const std::vector round{
        marker_instruction{ 2 },
        marker_instruction{ 3 }
    };

    definition_source_library sources;
    const auto [library, id_map] = sources.compile(initialization, round);
    const auto entry = detail::executor_access::entry(library);
    CHECK(detail::executor_access::instruction(library, entry).is<marker_instruction>());
    CHECK(detail::executor_access::instruction(library, entry).as<marker_instruction>().value == 1);
    CHECK(detail::executor_access::instruction(library, entry + 1).as<marker_instruction>().value == 4);
    CHECK(detail::executor_access::instruction(library, entry + 2).as<marker_instruction>().value == 2);
    CHECK(detail::executor_access::instruction(library, entry + 3).as<marker_instruction>().value == 3);

    const definition_selection selection{};
    const auto [selected_library, selected_id_map] = sources.compile(selection, initialization, round);
    const auto selected_entry = detail::executor_access::entry(selected_library);
    CHECK(detail::executor_access::instruction(selected_library, selected_entry).is<marker_instruction>());
    CHECK(
        detail::executor_access::instruction(selected_library, selected_entry + 3)
            .as<marker_instruction>().value == 3
    );
}
