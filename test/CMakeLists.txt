# Automatically enable catch2 to generate ctest targets
if (CONAN_CATCH2_ROOT_DEBUG)
    include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else ()
    include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif ()

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC CONAN_PKG::catch2)
target_link_libraries(catch_main PRIVATE project_options)


add_executable(tests)
target_link_libraries(tests PRIVATE project_warnings project_options catch_main)

target_include_directories(tests
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

# automatically discover tests that are defined in catch based test files you can modify the unittests. Set TEST_PREFIX
# to whatever you want, or use different for different binaries
catch_discover_tests(
        tests
        TEST_PREFIX
        "unittests."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "unittests."
        OUTPUT_SUFFIX
        .xml)

# Add a file containing a set of constexpr tests
add_executable(constexpr_tests)
target_link_libraries(constexpr_tests PRIVATE project_options project_warnings catch_main)

target_include_directories(constexpr_tests
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

catch_discover_tests(
        constexpr_tests
        TEST_PREFIX
        "constexpr."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "constexpr."
        OUTPUT_SUFFIX
        .xml)

# Disable the constexpr portion of the test, and build again this allows us to have an executable that we can debug when
# things go wrong with the constexpr testing
add_executable(relaxed_constexpr_tests)
target_link_libraries(relaxed_constexpr_tests PRIVATE project_options project_warnings catch_main)
target_compile_definitions(relaxed_constexpr_tests PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)

target_include_directories(relaxed_constexpr_tests
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

catch_discover_tests(
        relaxed_constexpr_tests
        TEST_PREFIX
        "relaxed_constexpr."
        REPORTER
        xml
        OUTPUT_DIR
        .
        OUTPUT_PREFIX
        "relaxed_constexpr."
        OUTPUT_SUFFIX
        .xml)

add_subdirectory(runtime)
add_subdirectory(constexpr)
