#include "ConfigPanel.h"

#include <imgui.h>

#include "Base/Config.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "GLFW/glfw3.h"


bool ConfigPanel::init(void *window, int width, int height) {
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

ConfigPanel::~ConfigPanel() {
    destroy();
}

void ConfigPanel::onDraw() {
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

void ConfigPanel::update() {
    // TODO:
}

void ConfigPanel::drawSettings() {
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
    ImGui::Checkbox("Use Mipmaps", &config.bUseMipmaps);

    ImGui::Separator();
    ImGui::InputFloat("Exposue", &config.Exposure, 0.1f, 1.0f, "%.2f");

    ImGui::Separator();
    const char* renderModes[] = { "Forward", "Deffered", "Test" };
    int currentPipeline = config.RenderPipeline;
    if (ImGui::Combo("Pipeline Type", &currentPipeline, renderModes, IM_ARRAYSIZE(renderModes))) {
        config.RenderPipeline = static_cast<ERenderPipeline>(currentPipeline);
    }

    if (config.RenderPipeline == RenderMode_DeferredRendering) {
        const char* shadingModels[] = { "Base Color", "Blinn-Phong", "PBR" };
        int currentModel = config.ShadingModelForDeferredRendering;
        if (ImGui::Combo("Shading For Deferred Rendering", &currentModel, shadingModels, IM_ARRAYSIZE(shadingModels))) {
            config.ShadingModelForDeferredRendering = static_cast<EShadingModel>(currentModel);
        }
        // if (ImGui::TreeNode("Shading For Deferred Rendering")) {
        //     ImGui::TreePop()
        // }
    }

    ImGui::Separator();
    const char* sceneType[] = {"Defualt", "StandfordBunny", "PBRTest"};
    int currentScene = config.SceneType;
    if (ImGui::Combo("Scene Type", &currentScene, sceneType, IM_ARRAYSIZE(sceneType))) {
        config.SceneType = static_cast<ESceneType>(currentScene);
    }

}

void ConfigPanel::updateSize(int width, int height) {
    frameWidth_ = width;
    frameHeight_ = height;
}

bool ConfigPanel::wantCaptureKeyboard() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool ConfigPanel::wantCaptureMouse() {
    ImGuiIO &io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool ConfigPanel::loadConfig() {
    return true;
}

bool ConfigPanel::reloadModel(const std::string &name) {
    return true;
}

bool ConfigPanel::reloadSkybox(const std::string &name) {
    return true;
}

void ConfigPanel::destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
