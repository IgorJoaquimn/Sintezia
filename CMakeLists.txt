cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "infinite-craft-clone")
project(${PROJECT_NAME})

# --- Find required packages ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
find_package(Freetype REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

# --- Add GLM and nlohmann_json using FetchContent ---
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# Add nlohmann_json for JSON support
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

# --- Collect source files ---
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/Math.cpp
    ${SRC_DIR}/Game/Game.cpp
    ${SRC_DIR}/Actor/Actor.cpp
    ${SRC_DIR}/Actor/TextActor.cpp
    ${SRC_DIR}/Actor/ItemActor.cpp
    ${SRC_DIR}/Actor/Player.cpp
    ${SRC_DIR}/Map/TileMap.cpp
    ${SRC_DIR}/Component/Component/Component.cpp
    ${SRC_DIR}/Shader/ShaderProgram.cpp
    ${SRC_DIR}/Font/FontManager.cpp
    ${SRC_DIR}/Core/Renderer/Renderer.cpp
    ${SRC_DIR}/Core/TextRenderer/TextRenderer.cpp
    ${SRC_DIR}/Core/RectRenderer/RectRenderer.cpp
    ${SRC_DIR}/Crafting/Item.cpp
    ${SRC_DIR}/Crafting/Crafting.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include directories ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# --- Link all necessary libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    GLEW::glew
    OpenGL::GL
    glm
    nlohmann_json::nlohmann_json
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Post-build commands and asset copying ---
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
