#include <utility>

#include <memory>

#include "Application.h"
#include "configLoader.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "dpi.h"
#include "res_fonts.h"
#include "IconsFontAwesome4.h"

using namespace sm;
using namespace sm::editor;

Application::Application() : projectWindow(this) {
    open(std::make_shared<project::Project>());
}

bool Application::init() {

    // Initialize imgui
    IMGUI_CHECKVERSION();
    ctx = ImGui::CreateContext();
    loader::installConfigLoader(ctx, this);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "showmaker.ini";
    ImGui::LoadIniSettingsFromDisk(io.IniFilename);
    if (windowWidth <= 100) windowWidth = 1280;
    if (windowHeight <= 100) windowHeight = 720;

    // Initialize glfw
    if (!glfwInit()) {
        printf("ERROR: Could not initialize glfw\n");
        return false;
    }

    // Make main window
    glfwWindowHint(GLFW_MAXIMIZED, showMaximized);
    mainWindow = glfwCreateWindow(windowWidth, windowHeight, APPL_NAME, nullptr, nullptr);
    if (!mainWindow) {
        printf("ERROR: Cannot create window\n");
        return false;
    }

    glfwMakeContextCurrent(mainWindow);
    glfwSwapInterval(1);

    // load opengl 3.0
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        printf("Cannot load OpenGL context\n");
        return false;
    }

    printf("Loaded OpenGL %i.%i\n", GLVersion.major, GLVersion.minor);
    if (GLVersion.major < 3) {
        printf("ERROR: OpenGL 3.0 required\n");
        return false;
    }

    ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
    ImGui_ImplOpenGL3_Init(nullptr);
    applyTheme();

    // some gl parameters
    glClearColor(0.3, 0.3, 0.3, 1);
    glClearDepth(1);

    device.open();

    return true;
}

void Application::applyTheme() {
    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();
    switch (theme) {
        case 0:
            ImGui::StyleColorsClassic();
            break;
        case 1:
            ImGui::StyleColorsDark();
            style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
            break;
        case 2:
            ImGui::StyleColorsLight();
            break;
    }
    style.FramePadding = {6, 4};
    style.WindowRounding = 3;
    style.ScrollbarRounding = 3;
    ImGuiIO &io = ImGui::GetIO();
    style.ScaleAllSizes(dpi);
    io.Fonts->Clear();
    // FONT_NORMAL
    io.FontDefault = loadFont(fonts::Roboto_Medium, fonts::Roboto_Medium_end, FONT_BASE * dpi, false);
    loadFont(fonts::fontawesome_webfont, fonts::fontawesome_webfont_end, FONT_BASE * dpi, true);
    // FONT_BIG
    loadFont(fonts::Roboto_Medium, fonts::Roboto_Medium_end, FONT_BASE * dpi * 2, false);
    loadFont(fonts::fontawesome_webfont, fonts::fontawesome_webfont_end, FONT_BASE * dpi * 2, true);
    dirtyStyle = false;
    ImGui_ImplOpenGL3_DestroyFontsTexture();
}

int Application::runLoop() {

    while (!exit && !glfwWindowShouldClose(mainWindow)) {
        // manage dpi change
        float currentDpi = getDpiForWindow(mainWindow);
        if (dpi != currentDpi) {
            dpi = currentDpi;
            dirtyStyle = true;
        }
        if (dirtyStyle) applyTheme();

        // process events
        glfwPollEvents();
        glfwMakeContextCurrent(mainWindow);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        int viewportWidth, viewportHeight;
        glfwGetFramebufferSize(mainWindow, &viewportWidth, &viewportHeight);

        // gui palette purpose
        ImGui::ShowDemoWindow();

        // show frame
        projectWindow.resize(viewportWidth, viewportHeight);
        projectWindow.showFrame();

        // Rendering
        ImGui::Render();

        glViewport(0, 0, viewportWidth, viewportHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        showMaximized = glfwGetWindowAttrib(mainWindow, GLFW_MAXIMIZED) != 0;
        if (!showMaximized) {
            windowWidth = viewportWidth;
            windowHeight = viewportHeight;
        }

        if (closeProject) {
            if (proj) {
                // some close behavior, for example save confirm
                proj.reset();
            }
            projectWindow.close();
            closeProject = false;
        }

        glfwSwapBuffers(mainWindow);
    }


    return 0;
}

void Application::cleanUp() {
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    if (ctx) ImGui::DestroyContext(ctx);
    ctx = nullptr;

    if (mainWindow) glfwDestroyWindow(mainWindow);
    mainWindow = nullptr;

    glfwTerminate();
}

void Application::open(std::shared_ptr<project::Project> _proj) {
    if (proj) return;
    proj = std::move(_proj);
    projectWindow.open(proj);
}

void Application::close() {
    closeProject = true;
}

ImFont *Application::loadFont(const char *start, const char *end, float size, bool fontAwesome) const {
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    const ImWchar *ranges = nullptr;
    if (fontAwesome) {
        config.MergeMode = true;
        config.GlyphMinAdvanceX = size;
        static const ImWchar icon_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
        ranges = icon_ranges;
    }
    return ImGui::GetIO().Fonts->AddFontFromMemoryTTF(
            (void *) start,
            (int) (end - start),
            size, &config, ranges);
}
