find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)

set(givm_example_generator "${CMAKE_CURRENT_SOURCE_DIR}/generate_example_tests.py")
set(givm_generated_example_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Scan at build time so document edits, additions and removals need no configure.
# The generator preserves the source timestamp when its contents are unchanged.
add_custom_target(
    givm_generate_example_tests
    COMMAND
        "${Python3_EXECUTABLE}" "${givm_example_generator}"
        --root "${PROJECT_SOURCE_DIR}"
        --output-dir "${givm_generated_example_dir}"
    BYPRODUCTS "${givm_generated_example_dir}/examples.cpp"
    COMMENT "Generating documentation example tests"
    VERBATIM
)
add_executable(givm_examples "${givm_generated_example_dir}/examples.cpp")
add_dependencies(givm_examples givm_generate_example_tests)
target_link_libraries(givm_examples PRIVATE givm::givm Catch2::Catch2)
target_include_directories(givm_examples PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
if(MSVC)
    target_compile_options(givm_examples PRIVATE /bigobj /utf-8 /we4715 /we4716)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(givm_examples PRIVATE -Werror=return-type)
endif()

catch_discover_tests(
    givm_examples
    TEST_PREFIX givm_example_
    ADD_TAGS_AS_LABELS
    PROPERTIES TIMEOUT 10
)
