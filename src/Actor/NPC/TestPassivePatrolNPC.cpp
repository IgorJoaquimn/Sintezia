#include "TestPassivePatrolNPC.hpp"
#include "../../Game/Game.hpp"

TestPassivePatrolNPC::TestPassivePatrolNPC(Game* game)
    : PatrolNPC(game, false)  // false = not aggressive
{
    // Configure sprite to use Player sprite sheet
    LoadSpriteSheet("assets/third_party/Cute_Fantasy_Free/Player/Player.png");
    SetSpriteConfiguration(32, 32, 6, 6, 8.0f);

    // Configure custom row mappings for Player.png sprite sheet
    // Layout: 0=idle_down, 1=idle_right, 2=idle_up, 3=walk_down, 4=walk_right, 5=walk_up
    // We'll reuse animations for left direction (flip right animations)
    SetIdleRows(0, 1, 1, 2);  // idle: down, left(use right), right, up
    SetWalkRows(3, 4, 4, 5);  // walk: down, left(use right), right, up

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

