#pragma once
#include <memory>
#include <string>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Renderer;
class FScene;
class Viewer;

class App
{
public:
    App(const App& app) = delete;
    App& operator=(const App& app) = delete;

    static App& getInstance();
    static float getDeltaTime();
    static std::shared_ptr<Renderer> getRenderer();

    // callbacks
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    static void mouse_click_callback(GLFWwindow* window, int button, int action, int mods);
    void processInput();

    // tools
    static void printMaxVertexAttributeNum();

    // setups
    void init();
    void setupWindow();
    void setupScene();
    void mainLoop();
    void terminate();

private:
    App() = default;

    std::string defaultConfigPath = "./configs/renderConfig.json";
    bool firstMouse = true;

    std::shared_ptr<Viewer> viewer_;
    std::shared_ptr<FScene> scene_;
    GLFWwindow* window = nullptr;
    float lastFrameTime = 0.0;
    float deltaTime_ = 0.01;

    float lastX = 0.f;
    float lastY = 0.f;
};
