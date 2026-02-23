#include "menu.h"
#include "dx11.h"
#include "../features/settings.h"
#include "../features/esp.h"
#include "imgui.h"
#include <cstdio>
#include <cmath>

void RenderMenu() {
    if (!g_ImGuiInitialized) return;

    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 520), ImGuiCond_FirstUseEver);
    ImGui::Begin("GFR Mod Menu [HOME to toggle]", &g_MenuVisible);

    ImGui::TextColored(ImVec4(1.0f, 0.75f, 0.35f, 1.0f), "Single menu (tabs merged)");
    ImGui::Spacing();

    ImGui::Text("=== Combat ===");
    ImGui::Checkbox("Silent Aim [F1]", &g_SilentAimEnabled);
    ImGui::Checkbox("Fast Bullet [F6]", &g_FastBullet);
    if (g_FastBullet) {
        ImGui::SliderFloat("Bullet Speed", &g_BulletSpeedMultiplier, 1.0f, 200.0f);
    }
    ImGui::Checkbox("Weakness Hit [F10]", &g_WeaknessHack);

    ImGui::Separator();
    ImGui::Text("=== Weapon Feel ===");
    ImGui::Checkbox("No Recoil [F4]", &g_NoRecoil);
    ImGui::Checkbox("No Spread [F5]", &g_NoSpread);

    ImGui::Separator();
    ImGui::Text("=== Resources ===");
    ImGui::Checkbox("Infinite Ammo [F2]", &g_InfiniteAmmo);
    ImGui::Checkbox("No Cooldown [F11]", &g_NoCooldown);

    ImGui::Separator();
    ImGui::Text("=== Movement ===");
    ImGui::Checkbox("Speed Boost [F3]", &g_SpeedBoost);
    if (g_SpeedBoost) {
        ImGui::SliderInt("Speed", &g_BoostedSpeed, 500, 2000);
    }

    ImGui::Separator();
    ImGui::Text("=== ESP ===");
    ImGui::Text("Hold ALT to show ESP");
    ImGui::Checkbox("Show NPC (Shop/Smith/etc)", &g_ShowNPCESP);
    ImGui::Text("Shows: Secret Rooms, Treasure/Event Box, NPC");
    AcquireSRWLockShared(&g_ESPLock);
    ImGui::Text("Detected: %zu objects", g_ESPObjects.size());
    ReleaseSRWLockShared(&g_ESPLock);

    ImGui::Separator();
    ImGui::Text("=== Other ===");
    ImGui::Text("Mouse Wheel = Auto Pickup");
    ImGui::Text("END = Exit Mod");

    ImGui::End();
}

// Helper: Draw icon shape based on ESP type
static void DrawESPIcon(ImDrawList* drawList, float cx, float cy, float size, ESPType type, ImU32 color, ImU32 outlineColor) {
    switch (type) {
    case ESPType::SecretWall:
        // Wall icon: brick pattern (rectangle with lines)
        drawList->AddRectFilled(ImVec2(cx - size, cy - size * 0.7f), ImVec2(cx + size, cy + size * 0.7f), color, 2.0f);
        drawList->AddRect(ImVec2(cx - size, cy - size * 0.7f), ImVec2(cx + size, cy + size * 0.7f), outlineColor, 2.0f, 0, 2.0f);
        drawList->AddLine(ImVec2(cx - size, cy), ImVec2(cx + size, cy), outlineColor, 1.5f);
        drawList->AddLine(ImVec2(cx, cy - size * 0.7f), ImVec2(cx, cy), outlineColor, 1.5f);
        drawList->AddLine(ImVec2(cx - size * 0.5f, cy), ImVec2(cx - size * 0.5f, cy + size * 0.7f), outlineColor, 1.5f);
        drawList->AddLine(ImVec2(cx + size * 0.5f, cy), ImVec2(cx + size * 0.5f, cy + size * 0.7f), outlineColor, 1.5f);
        break;

    case ESPType::SecretPortal:
        // Portal icon: diamond/rhombus with inner glow
        {
            ImVec2 diamond[4] = {
                ImVec2(cx, cy - size),
                ImVec2(cx + size, cy),
                ImVec2(cx, cy + size),
                ImVec2(cx - size, cy)
            };
            drawList->AddConvexPolyFilled(diamond, 4, color);
            drawList->AddPolyline(diamond, 4, outlineColor, ImDrawFlags_Closed, 2.5f);
            // Inner diamond
            float innerSize = size * 0.5f;
            ImVec2 inner[4] = {
                ImVec2(cx, cy - innerSize),
                ImVec2(cx + innerSize, cy),
                ImVec2(cx, cy + innerSize),
                ImVec2(cx - innerSize, cy)
            };
            drawList->AddPolyline(inner, 4, IM_COL32(255, 255, 255, 180), ImDrawFlags_Closed, 1.5f);
        }
        break;

    case ESPType::TreasureBox:
        // Treasure chest icon: rectangle with lid
        drawList->AddRectFilled(ImVec2(cx - size, cy - size * 0.3f), ImVec2(cx + size, cy + size * 0.7f), color, 3.0f);
        drawList->AddRect(ImVec2(cx - size, cy - size * 0.3f), ImVec2(cx + size, cy + size * 0.7f), outlineColor, 3.0f, 0, 2.0f);
        // Lid
        drawList->AddRectFilled(ImVec2(cx - size * 1.1f, cy - size * 0.7f), ImVec2(cx + size * 1.1f, cy - size * 0.3f), color, 2.0f);
        drawList->AddRect(ImVec2(cx - size * 1.1f, cy - size * 0.7f), ImVec2(cx + size * 1.1f, cy - size * 0.3f), outlineColor, 2.0f, 0, 2.0f);
        // Lock
        drawList->AddCircleFilled(ImVec2(cx, cy + size * 0.2f), size * 0.25f, IM_COL32(255, 215, 0, 255));
        break;

    case ESPType::EventBox:
        // Event icon: star shape
        {
            const int points = 5;
            float outerR = size;
            float innerR = size * 0.45f;
            ImVec2 star[10];
            for (int i = 0; i < 10; i++) {
                float r = (i % 2 == 0) ? outerR : innerR;
                float angle = (i * 36.0f - 90.0f) * 3.14159f / 180.0f;
                star[i] = ImVec2(cx + r * cosf(angle), cy + r * sinf(angle));
            }
            drawList->AddConvexPolyFilled(star, 10, color);
            drawList->AddPolyline(star, 10, outlineColor, ImDrawFlags_Closed, 2.0f);
        }
        break;

    case ESPType::NPC:
        // NPC icon: simple person marker
        drawList->AddCircleFilled(ImVec2(cx, cy - size * 0.45f), size * 0.35f, color, 18);
        drawList->AddCircle(ImVec2(cx, cy - size * 0.45f), size * 0.35f, outlineColor, 18, 2.0f);
        drawList->AddLine(ImVec2(cx, cy - size * 0.1f), ImVec2(cx, cy + size * 0.9f), outlineColor, 2.0f);
        drawList->AddLine(ImVec2(cx - size * 0.65f, cy + size * 0.2f), ImVec2(cx + size * 0.65f, cy + size * 0.2f), outlineColor, 2.0f);
        drawList->AddLine(ImVec2(cx, cy + size * 0.9f), ImVec2(cx - size * 0.5f, cy + size * 1.45f), outlineColor, 2.0f);
        drawList->AddLine(ImVec2(cx, cy + size * 0.9f), ImVec2(cx + size * 0.5f, cy + size * 1.45f), outlineColor, 2.0f);
        break;
    }
}

void RenderESPOverlay() {
    if (!g_ImGuiInitialized || !g_pRenderTargetView) return;

    RECT rect;
    GetClientRect(g_GameWindow, &rect);
    int screenWidth = rect.right - rect.left;
    int screenHeight = rect.bottom - rect.top;

    if (screenWidth <= 0 || screenHeight <= 0) return;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)screenWidth, (float)screenHeight));
    ImGui::Begin("ESP Overlay", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    AcquireSRWLockShared(&g_ESPLock);
    for (const auto& obj : g_ESPObjects) {
        Vector3 screenPos;
        if (WorldToScreen(obj.worldPos, screenPos, screenWidth, screenHeight)) {
            if (screenPos.x >= -50 && screenPos.x <= screenWidth + 50 &&
                screenPos.y >= -50 && screenPos.y <= screenHeight + 50) {

                // Scale icon size by distance (closer = bigger)
                float baseSize = 18.0f;
                float distScale = 1.0f / (obj.distance / 15.0f + 0.5f);
                float iconSize = fmaxf(12.0f, fminf(28.0f, baseSize * distScale));

                // Colors based on type
                ImU32 fillColor, outlineColor;
                switch (obj.type) {
                case ESPType::SecretWall:
                    fillColor = IM_COL32(65, 105, 225, 200);    // Royal Blue
                    outlineColor = IM_COL32(135, 206, 250, 255); // Light Sky Blue
                    break;
                case ESPType::SecretPortal:
                    fillColor = IM_COL32(138, 43, 226, 200);    // Blue Violet
                    outlineColor = IM_COL32(218, 112, 214, 255); // Orchid
                    break;
                case ESPType::TreasureBox:
                    fillColor = IM_COL32(34, 139, 34, 200);     // Forest Green
                    outlineColor = IM_COL32(144, 238, 144, 255); // Light Green
                    break;
                case ESPType::EventBox:
                    fillColor = IM_COL32(255, 165, 0, 200);     // Orange
                    outlineColor = IM_COL32(255, 215, 0, 255);  // Gold
                    break;
                case ESPType::NPC:
                    fillColor = IM_COL32(70, 130, 180, 200);    // Steel Blue
                    outlineColor = IM_COL32(173, 216, 230, 255); // Light Blue
                    break;
                default:
                    fillColor = IM_COL32(128, 128, 128, 200);
                    outlineColor = IM_COL32(200, 200, 200, 255);
                }

                // Draw icon
                DrawESPIcon(drawList, screenPos.x, screenPos.y, iconSize, obj.type, fillColor, outlineColor);

                // Draw label with distance (below icon)
                const char* label = obj.displayName.empty() ? "Object" : obj.displayName.c_str();
                char labelText[96];
                sprintf_s(labelText, "%s %.0fm", label, obj.distance);

                // Calculate text size for centering
                ImVec2 textSize = ImGui::CalcTextSize(labelText);
                float textX = screenPos.x - textSize.x * 0.5f;
                float textY = screenPos.y + iconSize + 4.0f;

                // Text shadow for readability
                drawList->AddText(ImVec2(textX + 1, textY + 1), IM_COL32(0, 0, 0, 200), labelText);
                drawList->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), labelText);
            }
        }
    }
    ReleaseSRWLockShared(&g_ESPLock);

    ImGui::End();

    // Compact info panel (top-left corner)
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowBgAlpha(0.6f);
    ImGui::Begin("##ESPInfo", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings);

    AcquireSRWLockShared(&g_ESPLock);
    size_t objCount = g_ESPObjects.size();
    ReleaseSRWLockShared(&g_ESPLock);

    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "ESP");
    ImGui::SameLine();
    ImGui::Text("| %zu", objCount);

    ImGui::End();
}
