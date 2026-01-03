#include "fov.h"
#include "settings.h"

GetFOV_t g_OriginalGetFOV = nullptr;
void* g_GetFOVAddr = nullptr;

float HookedGetFOV(void* camera) {
    g_CachedCamera = camera;
    
    float originalFOV = g_OriginalGetFOV ? g_OriginalGetFOV(camera) : 60.0f;
    
    if (g_OriginalFOV == 0.0f) {
        g_OriginalFOV = originalFOV;
    }
    
    if (g_FOVEnabled) {
        return g_CustomFOV;
    }
    
    return originalFOV;
}
