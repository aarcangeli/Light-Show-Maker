#include <utility>
#include <memory>
#include <unistd.h>
#include <string>
#include "Application.h"
#include "configLoader.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "dpi.h"
#include "res_fonts.h"
#include "IconsFontAwesome4.h"
#include "json/json.h"
#include <algorithm>
#include <fstream>
#include <streambuf>

using namespace sm;
using namespace sm::editor;

Application::Application() : projectWindow(this) {
}

bool Application::init() {

    // Initialize imgui
    IMGUI_CHECKVERSION();
    ctx = ImGui::CreateContext();
    loader::installConfigLoader(ctx, this);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = iniPath.c_str();
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

    // restore previous project
    load(autosavePath);
    if (!proj) {
        open(std::make_shared<project::Project>());
        proj->canvas.makeGroup()->addKey(0, 100);
        proj->canvas.makeGroup();
        proj->canvas.makeGroup();
    }

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
    style.WindowRounding = 0;
    style.WindowBorderSize = 0;
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
        projectWindow.dpi = dpi;
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
        //usleep(500 * 1000);

        // run commands
        for (auto &cmd : commands) {
            cmd.fn();
        }
        commands.resize(0);
    }

    save(autosavePath);

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

void Application::layerSelected(std::shared_ptr<project::LightGroup> layer) {
    selected = std::move(layer);
}

void Application::command(const std::string &name, const std::function<void()> &fn) {
    commands.push_back(AppCommand{name, fn});
}

void Application::setAppHome(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    size_t posDot = path.rfind('.');
    size_t posSlash = path.rfind('/');
    if (posSlash > 0 && posDot > posSlash) path.resize(posDot);

    home = std::move(path);
    iniPath = home + ".ini";
    autosavePath = home + "-autosave.json";
}

void Application::save(std::string filename) {
    Serializer<SER_JSON> serializer;
    proj->serialize(serializer);
    std::string data = serializer.toString();
    std::ofstream file(filename);
    file << data;
}

void Application::load(std::string filename) {
    std::ifstream file(filename);
    if (!file.good()) return;
    std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    Serializer<DESER_JSON> serializer(data);

    std::shared_ptr<project::Project> proj = std::make_shared<project::Project>();
    proj->serialize(serializer);
    open(proj);
}
