#include <memory>

#include "Application.h"
#include "configLoader.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

using namespace sm;
using namespace sm::editor;

Application::Application() {
    proj = std::make_shared<project::Project>();
}

bool Application::init() {

    // Initialize imgui
    IMGUI_CHECKVERSION();
    ctx = ImGui::CreateContext();
    loader::installConfigLoader(ctx, this);
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = "showmaker.ini";
    ImGui::LoadIniSettingsFromDisk(io.IniFilename);

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

    return true;
}

void Application::applyTheme() const {
    printf("APPLY\n");
    switch (theme) {
        case 0:
            ImGui::StyleColorsClassic();
            break;
        case 1:
            ImGui::StyleColorsDark();
            break;
        case 2:
            ImGui::StyleColorsLight();
            break;
    }
}

int Application::runLoop() {
    while (!exit && !glfwWindowShouldClose(mainWindow)) {
        // process events
        glfwPollEvents();
        glfwMakeContextCurrent(mainWindow);

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // gui palette purpose
        //ImGui::ShowDemoWindow();

        windowMenu();

        // show frame
        myEditor.editorOf(proj.get());

        // Rendering
        ImGui::Render();

        int viewportWidth, viewportHeight;
        glfwMakeContextCurrent(mainWindow);
        glfwGetFramebufferSize(mainWindow, &viewportWidth, &viewportHeight);
        glViewport(0, 0, viewportWidth, viewportHeight);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        showMaximized = glfwGetWindowAttrib(mainWindow, GLFW_MAXIMIZED) != 0;
        if (!showMaximized) {
            windowWidth = viewportWidth;
            windowHeight = viewportHeight;
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

void Application::windowMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                open(std::make_shared<project::Project>());
            }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // todo
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                // todo
            }
            if (ImGui::MenuItem("Save As..", "Ctrl+Shift+S")) {
                // todo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit")) {
                close();
                if (!proj) {
                    exit = true;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                // todo
            }
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {
                // todo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {
                // todo
            }
            if (ImGui::MenuItem("Copy", "CTRL+C")) {
                // todo
            }
            if (ImGui::MenuItem("Paste", "CTRL+V")) {
                // todo
            }
            ImGui::Separator();
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::Combo("Theme", &theme, "Classic\0Dark\0Light\0\0")) applyTheme();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Application::open(std::shared_ptr<project::Project> _proj) {
    close();
    if (proj) return;
    proj = _proj;
}

void Application::close() {
    if (!proj) return;
    // some close behavior, for example save confirm
    proj = nullptr;
}
