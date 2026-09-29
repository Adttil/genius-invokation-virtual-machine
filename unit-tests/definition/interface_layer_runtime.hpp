#ifndef GIVM_UNIT_TESTS_DEFINITION_INTERFACE_LAYER_RUNTIME_HPP
#define GIVM_UNIT_TESTS_DEFINITION_INTERFACE_LAYER_RUNTIME_HPP

#include <givm/runtime.hpp>

namespace givm_test::interface_layers
{
    givm::definition_library make_layered_library(givm::linked_deck& deck, bool observed);
}

#endif
