#include "TestAggressivePatrolNPC.hpp"
#include "../Game/Game.hpp"

TestAggressivePatrolNPC::TestAggressivePatrolNPC(Game* game)
    : PatrolNPC(game, true)  // true = aggressive
{
    // Configure sprite to use Player sprite sheet
    LoadSpriteSheet("assets/third_party/Cute_Fantasy_Free/Player/Player.png");
    SetSpriteConfiguration(32, 32, 6, 6, 8.0f);

    // Configure custom row mappings for Player.png sprite sheet
    // Layout: 0=idle_down, 1=idle_right, 2=idle_up, 3=walk_down, 4=walk_right, 5=walk_up
    // We'll reuse animations for left direction (flip right animations)
    SetIdleRows(0, 1, 1, 2);  // idle: down, left(use right), right, up
    SetWalkRows(3, 4, 4, 5);  // walk: down, left(use right), right, up

    // Configure attack animation rows (using walk animations as placeholder since Player.png doesn't have attack animations)
    // In a real sprite sheet with attack rows, you would use rows 6, 7, 8 as specified
    // attack: down, left, right, up
    SetAttackRows(6, 7, 7, 8);  // attack: down, left(use right), right, up

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

