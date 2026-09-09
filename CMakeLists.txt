cmake_minimum_required(VERSION 3.21)

project(
    givm
    DESCRIPTION "Genius Invokation Virtual Machine"
    LANGUAGES CXX
)

option(
    GIVM_BUILD_UNIT_TESTS
    "Build the GIVM unit tests"
    ${PROJECT_IS_TOP_LEVEL}
)
option(
    GIVM_BUILD_EXAMPLES
    "Build documentation example tests"
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

if(GIVM_BUILD_UNIT_TESTS OR GIVM_BUILD_EXAMPLES)
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.15.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
    include(Catch)
endif()

if(GIVM_BUILD_UNIT_TESTS)
    add_subdirectory(unit-tests)
endif()

if(GIVM_BUILD_EXAMPLES)
    add_subdirectory(example_test_tool)
endif()
