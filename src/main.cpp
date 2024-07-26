// learnOpenGL-aTriangle.cpp: 定义应用程序的入口点。
#include "Base/Globals.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <filesystem>
#include "Geometry/Primitives.h"
#include <memory>
#include "Utils/UniversalUtils.h"
#include "FCamera.h"
#include "Geometry/BufferAttribute.h"
#include "Geometry/Object.h"
#include "Material/FMaterial.h"
#include "Light.h"
#include "Geometry/Model.h"
#include "Utils/OpenGLUtils.h"
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
    fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
            type, severity, message);
}

class App
{
public:
    App(const App& app) = delete;
    App& operator=(const App& app) = delete;

    static App& getInstance();

    // callbacks
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    void processInput();

    // tools
    static void printMaxVertexAttributeNum();

    // setups
    void init();
    void setupWindow();
    void setupViewer();
    void setupScene();

    void mainLoop();
    void draw();

    void terminate();

private:
    App() = default;

    std::string defaultConfigPath = "./configs/renderConfig.json";
    bool firstMouse = true;

    std::shared_ptr<PerspectiveCamera> camera_;
    std::shared_ptr<Viewer> viewer_;
    std::shared_ptr<FScene> scene_;
    GLFWwindow* window = nullptr;
    float lastFrameTime = 0.0;
    float deltaFrameTime = 0.01;

    glm::vec3 cameraPos = glm::vec3(2.f, 0.0f, 2.0f);

    float lastX = 0.f;
    float lastY = 0.f;
};

App& App::getInstance() {
    static App app;
    return app;
}

void App::init() {
    setupWindow();
    setupViewer();
    setupScene();
    printMaxVertexAttributeNum();
}

void App::mainLoop() {
    // renderLoop
    int counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrameTime = glfwGetTime();
        deltaFrameTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        if (counter % 1000 == 55) {
            LOGI(("FPS: " + std::to_string(1.0f / deltaFrameTime)).c_str());
        }
        processInput();

        // render
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, 
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw();

        glfwSwapBuffers(window); // 交换(Swap)前缓冲和后缓冲
        glfwPollEvents();        // 检测事件
        counter = (counter + 1) % 100;
    }
}

void App::setupWindow() {
    // render config
    Config& config = Config::getInstance();
    config.deserialize(defaultConfigPath); // do this be fore any actual code
    std::cout << "Rendering Mode: " << static_cast<int>(config.RenderMode) << std::endl;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);                 // OpenGL 主版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // OpenGL 次版本号
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


    window = glfwCreateWindow(config.WindowWidth, config.WindowHeight, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(window); // 将我们窗口的上下文设置为当前线程的主上下文

    // 在调用任何OpenGL的函数之前我们需要初始化GLAD
    // glfwGRtProcAddress转为一个函数指针，函数指针返回*void
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    // 这些函数名字都是定义在glad的头文件里的
    // 左下角位置 宽 高
    int frameWidth, frameHeight;
    glfwGetFramebufferSize(window, &frameWidth, &frameHeight);
    std::cout << "Width: " << frameWidth << " Height:" << frameHeight << std::endl;
    glViewport(0, 0, frameWidth, frameHeight);

    // called when resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    //camera = std::make_shared<Camera>(0, 0, 5);
    camera_ = std::make_shared<PerspectiveCamera>(50.0f, static_cast<float>(frameWidth) / frameHeight);
    camera_->setPosition(cameraPos);
    // camera_->debugCamera();

    // viewer
    viewer_ = std::make_shared<Viewer>(camera_);
    viewer_->init(frameWidth, frameHeight, 0);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(openglCallbackFunction, nullptr);
}

void App::setupViewer() {
}

void App::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    App& app = getInstance();
    if (app.viewer_) {
        app.viewer_->setViewPort(0, 0, width, height);
    }
    else {
        glViewport(0, 0, width, height);
    }
    app.camera_->setAspect(static_cast<float>(width) / height);
}

void App::processInput()
{
    // check input key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 1.0f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_->ProcessKeyboard(FORWARD, deltaFrameTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_->ProcessKeyboard(BACKWARD, deltaFrameTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_->ProcessKeyboard(LEFT, deltaFrameTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_->ProcessKeyboard(RIGHT, deltaFrameTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera_->lookAt({0.f, 0.f, 0.f});
    }
}

// mouse callback
// ----------------------------------------------------------------------

void App::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    App& app = getInstance();
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
    app.camera_->ProcessMouseMovement(xoffset, yoffset);
}

// mouse scroll callback
// ----------------------------------------------------------------------

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    App& app = getInstance();
    app.camera_->ProcessMouseScroll(yoffset);;
}

void App::setupScene()
{
    scene_ = FScene::generateDeaultScene();

    // camera pos
    camera_->setPosition(-5, 5, -5);
    camera_->lookAt(scene_->getFocus());
}

void App::draw()
{
    viewer_->render(scene_);
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
    if (viewer_) {
        auto&& config = Config::getInstance();
        config.serialize(defaultConfigPath);
    }
    scene_ = nullptr;
    viewer_ = nullptr;
    glfwTerminate();
}

int main(){
    SerikaGL::Logger::setLogLevel(SerikaGL::LogLevel::LOG_DEBUG);
    App& app = App::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();
    return 0;
}