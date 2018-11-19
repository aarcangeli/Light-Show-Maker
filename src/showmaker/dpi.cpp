#include "dpi.h"

#ifdef WIN32

#define GLFW_EXPOSE_NATIVE_WIN32
#include "Windows.h"

#ifndef USER_DEFAULT_SCREEN_DPI
#define USER_DEFAULT_SCREEN_DPI 96
#endif

#ifndef DPI_ENUMS_DECLARED
typedef enum PROCESS_DPI_AWARENESS {
    PROCESS_DPI_UNAWARE = 0,
    PROCESS_SYSTEM_DPI_AWARE = 1,
    PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;

#endif

#include "GLFW/glfw3native.h"

typedef BOOL (WINAPI *SETPROCESSDPIAWARE_T)(void);
typedef HRESULT (WINAPI *SETPROCESSDPIAWARENESS_T)(PROCESS_DPI_AWARENESS);
typedef UINT (WINAPI *GETDPIFORWINDOW_T)(HWND);

static GETDPIFORWINDOW_T g_GetDpiForWindow;

#endif
namespace sm {

void setupDpi() {

#ifdef WIN32

    HMODULE shcore = LoadLibraryA("Shcore.dll");
    HMODULE user32 = LoadLibraryA("User32.dll");

    bool ok = false;

    if (shcore) {
        auto SetProcessDpiAwareness = (SETPROCESSDPIAWARENESS_T) GetProcAddress(shcore, "SetProcessDpiAwareness");
        if (SetProcessDpiAwareness) {
            SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
            ok = SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE) == S_OK;
        }
    }

    if (!ok && user32) {
        auto SetProcessDPIAware = (SETPROCESSDPIAWARE_T) GetProcAddress(user32, "SetProcessDPIAware");
        if (SetProcessDPIAware) {
            //SetProcessDPIAware();
        }
    }

    if (user32) {
        g_GetDpiForWindow = (GETDPIFORWINDOW_T) GetProcAddress(user32, "GetDpiForWindow");
    }

#endif
}

float getDpiForWindow(GLFWwindow *window) {

#ifdef WIN32
    if (g_GetDpiForWindow) {
        UINT ret = g_GetDpiForWindow(glfwGetWin32Window(window));
        if (ret) {
            return (float) ret / USER_DEFAULT_SCREEN_DPI;
        }
    }
#endif

    return 1.0f;
}

}
