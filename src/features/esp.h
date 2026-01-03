#pragma once
#include "../game/game.h"
#include <vector>

struct ESPObject {
    Vector3 worldPos;
    Vector3 screenPos;
    bool onScreen;
    float distance;
    bool isMonster;  // true = secret room (blue), false = treasure box (green)
};

extern std::vector<ESPObject> g_ESPObjects;
extern float g_ViewMatrix[16];
extern bool g_ViewMatrixValid;

// ESP functions
bool WorldToScreen(const Vector3& worldPos, Vector3& screenPos, int screenWidth, int screenHeight);
void MultiplyMatrix(const float* a, const float* b, float* result);
void UpdateViewMatrix();
void UpdateESPObjects();
