#include <utility>

#include <utility>
#include <memory>
#include <string>
#include <use_glad.h>
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
#include <nfd.h>
#include <set>

using namespace sm;
using namespace sm::editor;

namespace sm {
Application *gApp;
float dpi = 1;
}

Application::Application() {
    gApp = this;
}

bool Application::init() {

    home = Pathie::Path::exe().parent();
    iniPath = home / (Pathie::Path::exe().sub_ext(".ini").basename().str());
    iniPathStr = iniPath.str();
    autoSavePath = home / "autosave/autosave.lsmproj";

    // Initialize imgui
    IMGUI_CHECKVERSION();
    ctx = ImGui::CreateContext();
    loader::installConfigLoader(ctx, this);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = iniPathStr.c_str();
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

    glfwSetWindowFocusCallback(mainWindow, [](GLFWwindow *mainWindow, int focused){
        if (focused) {
            gApp->resourceManager.recheckAllResources();
        }
    });

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
    load(autoSavePath.str(), true);
    if (!proj) {
        open(std::make_shared<model::Project>());
        proj->canvas.makeGroup();
    }

    player.init();

    return true;
}

void Application::applyTheme() {
    ImGuiStyle &style = ImGui::GetStyle();
    style = ImGuiStyle();
    ImGui::StyleColorsDark();
    style.Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
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
        glfwSetWindowTitle(mainWindow, (std::string(APPL_NAME) + (filename.empty() ? "" : " - " + filename)).c_str());

        // manage dpi change
        float currentDpi = getDpiForWindow(mainWindow);
        if (dpi != currentDpi) {
            sm::dpi = currentDpi;
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

        // show frame
        projectWindow.resize(viewportWidth, viewportHeight);
        projectWindow.showFrame();

        // gui palette
        //ImGui::ShowDemoWindow();

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

        glfwSwapBuffers(mainWindow);
        //usleep(500 * 1000);

        history.update();
        hotKey.update();
        player.update(proj->canvas);
    }

    save(autoSavePath.str(), true);

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

void Application::open(std::shared_ptr<model::Project> _proj) {
    close();
    proj = std::move(_proj);
    projectWindow.open(proj);
    history.init(proj);
    selection.init(proj);
}

void Application::close() {
    if (proj) {
        // some close behavior, for example save confirm
        proj.reset();
    }
    projectWindow.close();
    filename = "";
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

bool Application::beginCommand(const std::string &name, bool mergeable) {
    return history.beginCommand(name, mergeable);
}

void Application::endCommand() {
    history.endCommand();
}

void Application::stopMerging() {
    history.stopMerging();
}

void Application::asyncCommand(const std::string &name, bool mergeable, const std::function<void()> &fn) {
    history.asyncCommand(name, mergeable, fn);
}

bool Application::save(std::string filename, bool quiet) {
    try {
        printf("Saving %s\n", filename.c_str());
        if (proj) {
            std::ofstream file(filename);
            if (!file.good()) return false;
            std::string data = serializeObject(proj, filename, false);
            file << data;
            exporter.exportIno(proj, filename);
            return true;
        }
        return false;
    } catch (const char *errorStr) {
        if (!quiet) error("Cannot open '" + filename + "'\nCause: " + errorStr);
        return false;
    } catch (...) {
        if (!quiet) error("Cannot open '" + filename + "'");
        return false;
    }
}

bool Application::load(std::string filename, bool quiet) {
    try {
        printf("Loading %s\n", filename.c_str());
        std::ifstream file(filename);
        if (!file.good()) {
            if (!quiet) error("Cannot open '" + filename + "'\nCause: " + strerror(errno));
            return false;
        }
        std::string data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        std::shared_ptr<model::Project> proj = deserializeObject<model::Project>(data, filename);
        open(proj);
        this->filename = filename;
        return true;
    } catch (const char *errorStr) {
        if (!quiet) error("Cannot open '" + filename + "'\nCause: " + errorStr);
        return false;
    } catch (...) {
        if (!quiet) error("Cannot open '" + filename + "'");
        return false;
    }
}

std::string Application::getPath(const std::string &pathes, bool isSave) {
    std::string path;
    nfdchar_t *outPath = nullptr;
    nfdresult_t result;
    if (isSave) {
        result = NFD_SaveDialog(pathes.c_str(), gApp->lastDirectory.c_str(), &outPath);
    } else {
        result = NFD_OpenDialog(pathes.c_str(), gApp->lastDirectory.c_str(), &outPath);
    }
    if (result == NFD_OKAY) {
        path = outPath;
        free(outPath);
    }
    return path;
}

void Application::saveLastDirectory(std::string path) {
    // remove last /
    int lastSlash = -1;
    for (int i = 0; path[i]; i++) if (path[i] == '/' || path[i] == '\\') lastSlash = i;
    if (lastSlash >= 0) {
        path[lastSlash] = '\0';
        lastDirectory = path;
    }
}

void Application::quit() {
    glfwSetWindowShouldClose(mainWindow, true);
}
