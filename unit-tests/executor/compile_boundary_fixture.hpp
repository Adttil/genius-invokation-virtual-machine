#ifndef GIVM_UNIT_TESTS_EXECUTOR_COMPILE_BOUNDARY_FIXTURE_HPP
#define GIVM_UNIT_TESTS_EXECUTOR_COMPILE_BOUNDARY_FIXTURE_HPP

#include <expected>
#include <vector>

#include <givm/executor.hpp>

namespace givm_test::executor::compile_boundary
{
    enum class sequence_form { span, array, vector, tuple, list, subset_variant, variadic };

    std::expected<givm::definition_compile_result, std::vector<givm::compile_error>> make_program(
        sequence_form form, givm::compile_mode mode);
}

#endif
