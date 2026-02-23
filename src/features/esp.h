#pragma once
#include "../game/game.h"
#include <string>
#include <vector>

enum class ESPType {
    SecretWall,      // Hidden breakable wall
    SecretPortal,    // Teleport portal to secret room
    TreasureBox,     // Treasure chest
    EventBox,        // Event/special box
    NPC              // Generic NPC (shop/smith/campfire/etc)
};

struct ESPObject {
    Vector3 worldPos;
    Vector3 screenPos;
    bool onScreen;
    float distance;
    ESPType type;
    std::string displayName;
};

extern std::vector<ESPObject> g_ESPObjects;
extern float g_ViewMatrix[16];
extern bool g_ViewMatrixValid;

// ESP functions
bool WorldToScreen(const Vector3& worldPos, Vector3& screenPos, int screenWidth, int screenHeight);
void MultiplyMatrix(const float* a, const float* b, float* result);
void UpdateViewMatrix();
void UpdateESPObjects();
