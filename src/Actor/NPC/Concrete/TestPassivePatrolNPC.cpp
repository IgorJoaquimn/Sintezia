#include "TestPassivePatrolNPC.hpp"
#include "../../../Game/Game.hpp"

TestPassivePatrolNPC::TestPassivePatrolNPC(Game* game)
    : PatrolNPC(game, false)  // false = not aggressive
{
    // Configure sprite to use Cavegirl sprite sheet from tsx
    LoadSpriteSheetFromTSX("assets/tiled/tilesets/Cavegirl.tsx");

    // Cavegirl sprite layout (column-based, now default):
    // Columns: 0=Down, 1=Up, 2=Left, 3=Right
    // Rows 0-3: Walk animation frames (4 frames total)
    // All defaults are now set for column-based animation

    // Set initial position and anchor
    SetPosition(Vector2(300.0f, 300.0f));
    SetAnchorPosition(Vector2(300.0f, 300.0f));

    // Configure movement
    SetMovementSpeed(80.0f);

    // Create a simple square patrol path
    AddWaypoint(Vector2(300.0f, 300.0f), 1.0f);  // Start position, wait 1 second
    AddWaypoint(Vector2(500.0f, 300.0f), 1.0f);  // Move right, wait 1 second
    AddWaypoint(Vector2(500.0f, 500.0f), 1.0f);  // Move down, wait 1 second
    AddWaypoint(Vector2(300.0f, 500.0f), 1.0f);  // Move left, wait 1 second
    // Loop back to first waypoint automatically
}

TestPassivePatrolNPC::~TestPassivePatrolNPC()
{
}

