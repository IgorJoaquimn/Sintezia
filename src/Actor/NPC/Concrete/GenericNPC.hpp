#pragma once
#include "../Base/DialogNPC.hpp"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class GenericNPC : public DialogNPC
{
public:
    GenericNPC(class Game* game, const json& npcData);
    virtual ~GenericNPC() = default;

private:
    void LoadFromJSON(const json& npcData);
    void SetupSprite(const std::string& spriteName);
    void SetupFaceset(const std::string& facesetPath);
};
