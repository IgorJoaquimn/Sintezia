#include "TestShopkeeperNPC.hpp"
#include "../../Game/Game.hpp"

TestShopkeeperNPC::TestShopkeeperNPC(Game* game)
    : DialogNPC(game)
{
    // Set position (can be adjusted as needed)
    SetPosition(Vector2(400.0f, 400.0f));

    // Configure sprite to use the same Player sprite sheet
    LoadSpriteSheet("assets/third_party/Cute_Fantasy_Free/Player/Player.png");
    SetSpriteConfiguration(32, 32, 6, 6, 8.0f);

    // Set greeting
    SetGreeting("Greetings, traveler! Welcome to my humble shop. How may I assist you today?");

    // Add two dialog options
    AddDialogOption(
        "Ask about the village",
        "Ah, our village! It's a peaceful place, nestled between the mountains and the sea. "
        "We have wonderful craftspeople here who can create amazing things from simple materials."
    );

    AddDialogOption(
        "Ask about rare items",
        "You're interested in rare items? Well, I've heard rumors of a legendary gem hidden "
        "deep in the caves to the north. They say it has magical properties that can enhance "
        "any crafted item!"
    );

    // Add first trade offer: Steam for Water + Fire
    TradeOffer trade1("Basic Alchemy: Create Steam", 5, 1); // Steam (id=5) x1
    trade1.AddRequirement(1, 1); // Water (id=1) x1
    trade1.AddRequirement(2, 1); // Fire (id=2) x1
    AddTradeOffer(trade1);

    // Add second trade offer: Lava for Earth + Fire
    TradeOffer trade2("Volcanic Creation: Create Lava", 6, 1); // Lava (id=6) x1
    trade2.AddRequirement(3, 1); // Earth (id=3) x1
    trade2.AddRequirement(2, 2); // Fire (id=2) x2
    AddTradeOffer(trade2);
}

TestShopkeeperNPC::~TestShopkeeperNPC()
{
}

