file(
    GLOB_RECURSE givm_unit_test_sources
    CONFIGURE_DEPENDS
    "*.cpp"
)
add_executable(givm_unit_tests ${givm_unit_test_sources})
target_link_libraries(
    givm_unit_tests
    PRIVATE
        givm::givm
        Catch2::Catch2WithMain
)

if(MSVC)
    target_compile_options(givm_unit_tests PRIVATE /bigobj)
endif()

catch_discover_tests(
    givm_unit_tests
    ADD_TAGS_AS_LABELS
    PROPERTIES TIMEOUT 60
)
