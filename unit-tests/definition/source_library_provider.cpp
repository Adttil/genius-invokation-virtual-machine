#include "source_library_provider.hpp"
#include "source_library_fixture.hpp"

#include <stdexcept>
#include <utility>

namespace givm_test::definition::source_library_linkage
{
    givm::definition_source_library make_source_closure()
    {
        auto result = givm::make_definition_source_library(shared_card, provider_card);
        if(not result) throw std::logic_error{ givm::error_string(result.error()) };
        return std::move(*result);
    }
}
