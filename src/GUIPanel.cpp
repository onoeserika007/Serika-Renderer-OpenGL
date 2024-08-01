#include "GUIPanel.h"

#include <imgui.h>

#include "Base/Config.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "GLFW/glfw3.h"


bool GUIPanel::init(void *window, int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGuiStyle *style = &ImGui::GetStyle();
    style->Alpha = 0.8f;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL((GLFWwindow *) window, true);
    ImGui_ImplOpenGL3_Init("#version 430 core");

    return true;
}

GUIPanel::~GUIPanel() {
    destroy();
}

void GUIPanel::onDraw() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Settings window
    ImGui::Begin("Settings",
                 nullptr,
                 ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_AlwaysAutoResize);
    drawSettings();
    ImGui::SetWindowPos(ImVec2(frameWidth_ - ImGui::GetWindowWidth(), 0));
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIPanel::update() {
    // TODO:
}

void GUIPanel::drawSettings() {
    auto&& config = Config::getInstance();
    // fps
    ImGui::Separator();
    ImGui::Text("fps: %.1f (%.2f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);

    ImGui::Separator();
    ImGui::Checkbox("Use Tone Mapping", &config.bUseHDR);

    ImGui::Separator();
    ImGui::Checkbox("Use SSAO", &config.bUseSSAO);

    ImGui::Separator();
    ImGui::Checkbox("Use ShadowMap", &config.bShadowMap);

    ImGui::Separator();
    ImGui::InputFloat("Exposue", &config.Exposure, 0.1f, 1.0f, "%.2f");

    ImGui::Separator();
    const char* renderModes[] = { "Forward", "Deffered", "Test" };
    int currentMode = config.RenderMode;
    if (ImGui::Combo("Pipeline Type", &currentMode, renderModes, IM_ARRAYSIZE(renderModes))) {
        config.RenderMode = static_cast<ERenderMode>(currentMode);
    }

    ImGui::Separator();
    const char* sceneType[] = {"Defualt", "StandfordBunny", "PBRTest"};
    int currentScene = config.SceneType;
    if (ImGui::Combo("Scene Type", &currentScene, sceneType, IM_ARRAYSIZE(sceneType))) {
        config.SceneType = static_cast<ESceneType>(currentScene);
    }

}

void GUIPanel::updateSize(int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;
}

bool GUIPanel::wantCaptureKeyboard() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool GUIPanel::wantCaptureMouse() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool GUIPanel::loadConfig() {
    return true;
}

bool GUIPanel::reloadModel(const std::string &name) {
    return true;
}

bool GUIPanel::reloadSkybox(const std::string &name) {
    return true;
}

void GUIPanel::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
