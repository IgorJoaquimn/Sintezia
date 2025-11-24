#include "TestAggressivePatrolNPC.hpp"
#include "../Game/Game.hpp"

TestAggressivePatrolNPC::TestAggressivePatrolNPC(Game* game)
    : PatrolNPC(game, true)  // true = aggressive
{
    // Configure sprite to use the same Player sprite sheet
    LoadSpriteSheet("assets/third_party/Cute_Fantasy_Free/Player/Player.png");
    SetSpriteConfiguration(32, 32, 6, 6, 8.0f);

    // Set initial position and anchor
    SetPosition(Vector2(700.0f, 300.0f));
    SetAnchorPosition(Vector2(700.0f, 300.0f));

    // Configure movement
    SetMovementSpeed(60.0f);   // Slower patrol speed
    SetChaseSpeed(120.0f);     // Faster chase speed

    // Configure aggression ranges
    SetAggroRange(150.0f);     // Start chasing when player is within 150 units
    SetDeaggroRange(400.0f);   // Stop chasing when NPC is 400 units from anchor
    SetMaxChaseDistance(250.0f); // Stop chasing if player gets more than 250 units away from NPC

    // Create a simple back-and-forth patrol path
    AddWaypoint(Vector2(700.0f, 300.0f), 2.0f);  // Start position, wait 2 seconds
    AddWaypoint(Vector2(700.0f, 500.0f), 2.0f);  // Move down, wait 2 seconds
    // Loop back to first waypoint automatically
}

TestAggressivePatrolNPC::~TestAggressivePatrolNPC()
{
}

