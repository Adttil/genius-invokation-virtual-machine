#include <tuple>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <givm/definition/source_library.hpp>

using namespace givm;

namespace
{
    struct marker_instruction
    {
        using context_type = void;

        int value;

        bool execute(card_table&, execution_context&, random_fn&) const noexcept
        {
            return value != 0;
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
    const auto entry = library.entry();
    CHECK(library.instruction(entry).is<marker_instruction>());
    CHECK(library.instruction(entry).as<marker_instruction>().value == 1);
    CHECK(library.instruction(entry + 1).as<marker_instruction>().value == 4);
    CHECK(library.instruction(entry + 2).as<marker_instruction>().value == 2);
    CHECK(library.instruction(entry + 3).as<marker_instruction>().value == 3);

    const definition_selection selection{};
    const auto [selected_library, selected_id_map] = sources.compile(selection, initialization, round);
    const auto selected_entry = selected_library.entry();
    CHECK(selected_library.instruction(selected_entry).is<marker_instruction>());
    CHECK(
        selected_library.instruction(selected_entry + 3)
            .as<marker_instruction>().value == 3
    );
}
