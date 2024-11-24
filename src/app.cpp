#include "app.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include "FCamera.h"
#include "Geometry/BufferAttribute.h"
#include "Geometry/Object.h"
#include "ULight.h"
#include "Base/Config.h"
#include "FScene.h"
#include "Viewer.h"
#include "Base/ResourceLoader.h"

using namespace std;

void APIENTRY openglCallbackFunction(GLenum source,
                                     GLenum type,
                                     GLuint id,
                                     GLenum severity,
                                     GLsizei length,
                                     const GLchar *message,
                                     const void *userParam)
{
    char info[512];
    snprintf(info, sizeof(info), "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
    // 打开文件
    std::ofstream file("error_log.txt", std::ios_base::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for logging" << std::endl;
        return;
    }
    // 保存原来的缓冲区
    std::streambuf* originalCerrBuffer = std::cerr.rdbuf();
    // 重定向 std::cerr 到文件
    std::cerr.rdbuf(file.rdbuf());

    // 现在所有的 std::cerr 输出都会写入到 error_log.txt
    std::cerr << "Debug message (" << id << "): " << message << std::endl;

    std::cerr << info << std::endl;
    // 恢复原来的缓冲区
    std::cerr.rdbuf(originalCerrBuffer);
}
App& App::getInstance() {
    static App app;
    return app;
}

float App::getDeltaTime() {
    auto&& app = App::getInstance();
    return app.deltaTime_;
}

std::shared_ptr<Renderer> App::getRenderer() {
    auto&& app = App::getInstance();
    if (app.viewer_) {
        return app.viewer_->getRenderer();
    }
    return {};
}

void App::init() {
    setupWindow();
    setupScene();
    // printMaxVertexAttributeNum();
}

void App::mainLoop() {
    // renderLoop
    int counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaTime_ = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        if (counter % 1000 == 55) {
            LOGI(("FPS: " + std::to_string(1.0f / deltaTime_)).c_str());
        }
        processInput();;

        viewer_->DrawFrame();
        glfwSwapBuffers(window); // 交换(Swap)前缓冲和后缓冲
        glfwPollEvents();        // 检测事件
        counter = (counter + 1) % 100;
    }
}

void App::setupWindow() {
    // render config
    Config& config = Config::getInstance();
    // config.deserialize(defaultConfigPath);
    std::cout << "Rendering Mode: " << static_cast<int>(config.RenderPipeline) << std::endl;
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                 // OpenGL 主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // OpenGL 次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    window = glfwCreateWindow(config.WindowWidth, config.WindowHeight, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window); // 将我们窗口的上下文设置为当前线程的主上下文

    // 在调用任何OpenGL的函数之前我们需要初始化GLAD
    // glfwGRtProcAddress转为一个函数指针，函数指针返回*void
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_move_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_click_callback);

    // viewer
    viewer_ = std::make_shared<Viewer>();
    *viewer_;
    // glViewport在render的setViewport里调用
    viewer_->init(window, config.WindowWidth, config.WindowHeight, 0);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglCallbackFunction, nullptr);
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    App& app = getInstance();
    if (app.viewer_) {
        app.viewer_->setViewPort(0, 0, width, height);
        if (auto&& camera = app.viewer_->getViewCamera()) {
            camera->setAspect(static_cast<float>(width) / height);
        }
    }

}

void App::processInput() {
    // check input key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (viewer_) {
        viewer_->listenKeyEvents();
    }
}

// mouse callback
// ----------------------------------------------------------------------

void App::mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
    App& app = getInstance();
    if (!app.viewer_ || app.viewer_->wantCaptureMouse()) return;
    if (app.firstMouse) // 这个bool变量初始时是设定为true的
    {
        app.lastX = xpos;
        app.lastY = ypos;
        app.firstMouse = false;
    }

    float xoffset = xpos - app.lastX;
    float yoffset = app.lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
    app.lastX = xpos;
    app.lastY = ypos;

    const bool bUseFPS = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    const bool bUseOrbit = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    if (bUseFPS) app.viewer_->setCameraMode(CameraMode_FPSCamera);
    else if (bUseOrbit) app.viewer_->setCameraMode(CameraMode_OrbitCamera);
    else app.viewer_->setCameraMode(CameraMode_None);

    if (!bUseFPS && bUseOrbit) {
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            app.viewer_->updateOrbitPan(xoffset, yoffset);
        } else {
            app.viewer_->updateOrbitRotate(xoffset, yoffset);
        }
    }
    else if (bUseFPS && !bUseOrbit) {
        if (auto&& camera = app.viewer_->getViewCamera()) {
            camera->ProcessMouseMovement(xoffset, yoffset);
        }
    }

}

// mouse scroll callback
// ----------------------------------------------------------------------

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    App& app = getInstance();
    if (!app.viewer_ || app.viewer_->wantCaptureMouse()) return;
    if (auto&& camera = app.viewer_->getViewCamera()) {
        camera->ProcessMouseScroll(yoffset);;
    }
}

// mouse click callback
// ----------------------------------------------------------------------

void App::mouse_click_callback(GLFWwindow *window, int button, int action, int mods) {
    App& app = getInstance();
    if (!app.viewer_ || app.viewer_->wantCaptureMouse()) return;
    const bool bCtrlPressed = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && bCtrlPressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        const int shootX = xpos;
        const int shootY = ypos;

        app.viewer_->drawCursorHitDebugLine(shootX, shootY, width, height);
        app.viewer_->drawUnderCursorTraceDebugTriangle(shootX, shootY, width, height);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // back to fps, reset first mouse
        app.firstMouse = true;
        app.viewer_->setCameraMode(CameraMode_FPSCamera);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        app.viewer_->setCameraMode(CameraMode_None);;
    }

}

void App::setupScene()
{
    // if (auto&& camera = viewer_->getViewCamera()) {
    //     // default camera pos
    //     camera->setPosition(2, 2, 2);;
    //     // scene_ = FScene::generateDeaultScene(camera);
    //     // scene_ = FScene::generateRaytracingStanfordBunnyScene(camera);
    //     // scene_ = FScene::generateRaytracingCornellBoxScene(camera_);
    //     scene_ = FScene::generatePBRScene(camera);
    //     scene_->setupScene();
    //     if (scene_) {
    //         camera->lookAt(scene_->getFocus());
    //     }
    //     viewer_->setScene(scene_);
    // }
}

void App::printMaxVertexAttributeNum() {
    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
}

void App::terminate() {
    // 先析构Renderer中的UniformBlock 但是有8个引用，资源肯定是泄露了
    // 不是基类虚函数的问题，智能指针可以解决这个问题
    // 是object也和app一个生命周期，app直到进程退出才被销毁
    // renderer = nullptr;
    // 释放资源
    scene_ = nullptr;
    viewer_ = nullptr;
    glfwTerminate();
}


