cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME "infinite-craft-clone")
project(${PROJECT_NAME})

# --- Find required packages ---
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)
pkg_check_modules(SDL2_mixer REQUIRED SDL2_mixer)
pkg_check_modules(SDL2_ttf REQUIRED SDL2_ttf)
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
    ${SRC_DIR}/MathUtils.cpp
    ${SRC_DIR}/Game/Game.cpp
    ${SRC_DIR}/Game/Inventory.cpp
    ${SRC_DIR}/Game/ItemGenerator.cpp
    ${SRC_DIR}/Actor/Actor.cpp
    ${SRC_DIR}/Actor/TextActor.cpp
    ${SRC_DIR}/Actor/ItemActor.cpp
    ${SRC_DIR}/Actor/Player.cpp
    ${SRC_DIR}/Actor/NPC/Base/NPC.cpp
    ${SRC_DIR}/Actor/NPC/Base/DialogNPC.cpp
    ${SRC_DIR}/Actor/NPC/Base/PatrolNPC.cpp
    ${SRC_DIR}/Actor/NPC/Base/PassiveAnimalNPC.cpp
    ${SRC_DIR}/Actor/NPC/Base/AggressiveNPC.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Aggressive/Skeleton.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Aggressive/Flam.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Aggressive/GoldenStatue.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Aggressive/Shaman.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Aggressive/Spirit.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Aggressive/Statue.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Passive/CatNPC.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Passive/CowNPC.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Passive/ChickenNPC.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Passive/PigNPC.cpp
    ${SRC_DIR}/Actor/NPC/Concrete/Passive/GenericNPC.cpp
    ${SRC_DIR}/Map/TileMap.cpp
    ${SRC_DIR}/Map/TiledParser.cpp
    ${SRC_DIR}/Component/Component.cpp
    ${SRC_DIR}/Component/PlayerInputComponent.cpp
    ${SRC_DIR}/Component/MovementComponent.cpp
    ${SRC_DIR}/Component/AnimationComponent.cpp
    ${SRC_DIR}/Component/SpriteComponent.cpp
    ${SRC_DIR}/Component/HealthComponent.cpp
    ${SRC_DIR}/Component/AttackComponent.cpp
    ${SRC_DIR}/Component/HungerComponent.cpp
    ${SRC_DIR}/Component/ThirstComponent.cpp
    ${SRC_DIR}/Shader/ShaderProgram.cpp
    ${SRC_DIR}/Font/FontManager.cpp
    ${SRC_DIR}/Core/Renderer/Renderer.cpp
    ${SRC_DIR}/Core/TextRenderer/TextRenderer.cpp
    ${SRC_DIR}/Core/RectRenderer/RectRenderer.cpp
    ${SRC_DIR}/Core/Texture/Texture.cpp
    ${SRC_DIR}/Core/Texture/SpriteRenderer.cpp
    ${SRC_DIR}/Core/Camera.cpp
    ${SRC_DIR}/Crafting/Item.cpp
    ${SRC_DIR}/Crafting/Crafting.cpp
    ${SRC_DIR}/UI/NPCDialogUI.cpp
    ${SRC_DIR}/UI/InventoryUI.cpp
    ${SRC_DIR}/UI/MainMenu.cpp
    ${SRC_DIR}/UI/WarningPopup.cpp
    ${SRC_DIR}/UI/ControlsUI.cpp
    ${SRC_DIR}/UI/GameOverUI.cpp
        src/UI/HealthBar.hpp
        src/UI/HealthBar.cpp
        src/AudioSystem/AudioSystem.cpp
        src/AudioSystem/AudioSystem.h

)

add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include directories ---
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_mixer_INCLUDE_DIRS}
    ${SDL2_ttf_INCLUDE_DIRS}
    ${FREETYPE_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
)

# --- Link all necessary libraries ---
target_link_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_IMAGE_LIBRARY_DIRS}
    ${SDL2_mixer_LIBRARY_DIRS}
    ${SDL2_ttf_LIBRARY_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_mixer_LIBRARIES}
    ${SDL2_ttf_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    GLEW::glew
    OpenGL::GL
    glm
    nlohmann_json::nlohmann_json
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Post-build commands and asset copying ---
# Copy shaders on every build
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# Create a custom target to copy assets that runs on every build
# This ensures assets are updated even if source code hasn't changed
add_custom_target(CopyAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMENT "Copying assets to build directory..."
)

# Make sure the project depends on CopyAssets
add_dependencies(${PROJECT_NAME} CopyAssets)
