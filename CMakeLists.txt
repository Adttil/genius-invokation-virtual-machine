cmake_minimum_required(VERSION 3.21)

project(
    givm
    DESCRIPTION "Genius Invokation Virtual Machine"
    LANGUAGES CXX
)

option(
    GIVM_BUILD_SMOKE_TEST
    "Build the minimal game-flow smoke test"
    ${PROJECT_IS_TOP_LEVEL}
)
option(
    GIVM_BUILD_UNIT_TESTS
    "Build the GIVM unit tests"
    ${PROJECT_IS_TOP_LEVEL}
)

add_library(givm INTERFACE)
add_library(givm::givm ALIAS givm)
target_compile_features(givm INTERFACE cxx_std_23)
target_include_directories(
    givm
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)

if(GIVM_BUILD_SMOKE_TEST)
    add_executable(givm_smoke_test "test.cpp")
    target_link_libraries(givm_smoke_test PRIVATE givm::givm)

    if(MSVC)
        target_compile_options(givm_smoke_test PRIVATE /bigobj)
    endif()
endif()

if(GIVM_BUILD_UNIT_TESTS)
    enable_testing()
    add_subdirectory(unit-tests)
endif()
