
add_executable(example)

target_sources(
        example
        PRIVATE
            main.cpp
)

target_include_directories(example
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

target_link_libraries(
        example
        PRIVATE
        project_options
        project_warnings
)