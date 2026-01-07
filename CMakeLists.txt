cmake_minimum_required(VERSION 3.20)
project(nvapi-cli LANGUAGES CXX RC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB CLI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/cli/*.cpp"
)

add_executable(nvapi-cli
    src/nvapi-cli.cpp
    src/nvapi-cli.rc
    ${CLI_SOURCES}
)

target_include_directories(nvapi-cli PRIVATE
    "${CMAKE_SOURCE_DIR}/include"
)

target_compile_definitions(nvapi-cli PRIVATE
    WIN32
    NOMINMAX
)

if(MSVC)
    target_compile_options(nvapi-cli PRIVATE /W4 /EHsc)
    set_source_files_properties(src/nvapi-cli.rc PROPERTIES
        COMPILE_FLAGS "/I\"${CMAKE_SOURCE_DIR}/src\""
    )
endif()

target_link_libraries(nvapi-cli PRIVATE
    "${CMAKE_SOURCE_DIR}/lib/nvapi64.lib"
    d3d11
    dxgi
    opengl32
    user32
    gdi32
)
