#ifndef GIVM_UNIT_TESTS_DEFINITION_SOURCE_LIBRARY_PROVIDER_HPP
#define GIVM_UNIT_TESTS_DEFINITION_SOURCE_LIBRARY_PROVIDER_HPP

#include <givm/source_library.hpp>

namespace givm_test::definition::source_library_linkage
{
    givm::definition_source_library make_source_closure();
    givm::definition_source_library make_overlapping_closure();
    givm::definition_source_library make_conflicting_closure();
}

#endif
