add_executable(givm_unit_tests
    tests.cpp
    executor/compile_boundary_fixture.cpp
    definition/source_library_provider.cpp
    definition/source_library_peer.cpp
    definition/source_library_client.cpp
    definition/interface_layer_author.cpp
    definition/interface_layer_compile.cpp
    definition/interface_layer_client.cpp
)
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
