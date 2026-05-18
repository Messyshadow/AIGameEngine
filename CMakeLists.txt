cmake_minimum_required(VERSION 3.20)
project(AIForge LANGUAGES C CXX)

# ============================================================================
# Global config
# ============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 11)

# Put all binaries in build/bin so we can find chapter_XX_demo.exe easily
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(MSVC)
    add_compile_options(/utf-8 /W3 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# ============================================================================
# Third-party deps
# ============================================================================
include(FetchContent)

# --- SDL3 ---
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS        OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL      OFF CACHE BOOL "" FORCE)
set(SDL_SHARED       ON  CACHE BOOL "" FORCE)
set(SDL_STATIC       OFF CACHE BOOL "" FORCE)
message(STATUS "[AIForge] Fetching SDL3 (release-3.2.0)...")
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.2.0
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(SDL3)

# --- nlohmann_json (header-only, single-header) ---
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
message(STATUS "[AIForge] Fetching nlohmann_json (v3.11.3)...")
FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

# ============================================================================
# glad — OpenGL function loader (vendored from depends/glad/)
# ============================================================================
add_library(glad STATIC depends/glad/src/glad.c)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/depends/glad/include
)

# ============================================================================
# aiforge — engine core static lib
#   contains everything in engine/core/ + engine/command/
# ============================================================================
file(GLOB_RECURSE AIFORGE_SRC CONFIGURE_DEPENDS
    engine/core/*.cpp
    engine/core/*.h
    engine/command/*.cpp
    engine/command/*.h
    engine/render/*.cpp
    engine/render/*.h
)
add_library(aiforge STATIC ${AIFORGE_SRC})
target_include_directories(aiforge PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/depends/stb
)
target_link_libraries(aiforge PUBLIC
    SDL3::SDL3
    glad
    nlohmann_json::nlohmann_json
)
if(WIN32)
    target_link_libraries(aiforge PUBLIC opengl32)
elseif(APPLE)
    target_link_libraries(aiforge PUBLIC "-framework OpenGL")
else()
    target_link_libraries(aiforge PUBLIC GL ${CMAKE_DL_LIBS})
endif()

# ============================================================================
# Helper: define a chapter demo target with consistent linkage + DLL copy.
# Usage: aiforge_chapter_demo(<target> <main.cpp>)
# ============================================================================
function(aiforge_chapter_demo target_name source_file)
    add_executable(${target_name} ${source_file})
    target_link_libraries(${target_name} PRIVATE aiforge)
    if(WIN32)
        add_custom_command(TARGET ${target_name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                $<TARGET_FILE:SDL3::SDL3>
                $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copying SDL3.dll next to ${target_name}"
        )
    endif()
    # 把项目的 data/ 同步到 .exe 旁边,让相对路径 "data/..." 能工作
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/data
            $<TARGET_FILE_DIR:${target_name}>/data
        COMMENT "Syncing data/ next to ${target_name}"
    )
endfunction()

# ============================================================================
# Chapter demos — each one: examples/XX_name/main.cpp -> chapter_XX_demo.exe
# ============================================================================
aiforge_chapter_demo(chapter_01_demo examples/01_hello_window/main.cpp)
aiforge_chapter_demo(chapter_02_demo examples/02_ecs_demo/main.cpp)
aiforge_chapter_demo(chapter_03_demo examples/03_command_demo/main.cpp)
aiforge_chapter_demo(chapter_04_demo examples/04_2d_sprite_batcher/main.cpp)
aiforge_chapter_demo(chapter_05_demo examples/05_2d_animation_particles_tilemap/main.cpp)
aiforge_chapter_demo(chapter_06_demo examples/06_2d_lighting_postfx/main.cpp)
aiforge_chapter_demo(chapter_07_demo examples/07_3d_mesh_camera/main.cpp)
aiforge_chapter_demo(chapter_08_demo examples/08_pbr_materials_lighting/main.cpp)

# ============================================================================
# Status summary
# ============================================================================
message(STATUS "[AIForge] Configured. Build with:")
message(STATUS "[AIForge]   cmake --build ${CMAKE_BINARY_DIR} --config Release")
message(STATUS "[AIForge] Then run: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/chapter_01_demo.exe")
