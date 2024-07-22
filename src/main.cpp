// learnOpenGL-aTriangle.cpp: 定义应用程序的入口点。
#include "Base/GLMInc.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "primitives.h"
#include <filesystem>
#include <vector>
#include <memory>
#include "Utils/utils.h"
#include "Shader.h"
#include "../include/OpenGL/ShaderGLSL.h"
#include "Camera.h"
#include "Geometry/Geometry.h"
#include "BufferAttribute.h"
#include "../include/Geometry/Object.h"
#include "Material.h"
#include "Light.h"
#include "../include/Geometry/Model.h"
#include "../include/OpenGL/ViewerOpenGL.h"
#include "Utils/OpenGLUtils.h"
#include "Base/Config.h"
#include "Scene.h"
#include "Base/ResourceLoader.h"

using namespace std;

class App
{
    std::string defaultConfigPath = "./renderConfig.json";
    float mixValue = 0.5;
    bool firstMouse = true;

    int windowWidth = 0;
    int windowHeight = 0;
    std::shared_ptr<UObject> cube {};
    std::shared_ptr<UObject> lightMapCube {};
    std::vector<std::shared_ptr<ULight>> lightArray {};
    std::shared_ptr<PerspectiveCamera> camera;
    std::shared_ptr<Viewer> viewer_;
    std::shared_ptr<Scene> scene_;
    GLFWwindow* window = nullptr;
    float lastFrameTime = 0.0;
    float deltaFrameTime = 0.01;

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    glm::vec3 cameraPos = glm::vec3(2.f, 0.0f, 2.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float lastX = 0.f;
    float lastY = 0.f;
    int UUID = -1;
    App() = default;
public:
    App(const App& app) = delete;
    App& operator=(const App& app) = delete;
    static App& getInstance();

    void init();

    void mainLoop();

    void setupWindow();

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

    void processInput();

    // mouse callback
    // ----------------------------------------------------------------------
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    // mouse scroll callback
    // ----------------------------------------------------------------------
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    void setupScene();

    void beforeLoop();

    void draw();

    static void printMaxVertexAttributeNum();

    void terminate();

    static void printMat4(const string& name, glm::mat4 mat);

};

int main(){
    SerikaGL::Logger::setLogLevel(SerikaGL::LogLevel::LOG_DEBUG);
    App& app = App::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();
    return 0;
}

App& App::getInstance() {
    static App app;
    return app;
}

void App::init() {
    setupWindow();
    setupScene();
    printMaxVertexAttributeNum();
    beforeLoop();
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
    //glGetError();
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

    // camera
    //camera = std::make_shared<Camera>(0, 0, 5);
    camera = std::make_shared<PerspectiveCamera>(50.0f, static_cast<float>(frameWidth) / frameHeight);
    camera->setPosition(cameraPos);

    // viewer
    viewer_ = std::make_shared<ViewerOpenGL>(camera);
    viewer_->init(frameWidth, frameHeight, 0);
    //viewer_ = nullptr;
    //ViewerOpenGL testViewer{ *camera, config };

    // called when resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
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
    app.camera->setAspect(static_cast<float>(width) / height);
}

void App::processInput()
{
    // check input key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        mixValue += 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue >= 1.0f)
            mixValue = 1.0f;
        // 应该是float！！
        //std::cout << "GLFW_KEY_UP PRESSED!" << std::endl;
        //std::cout << "MixValue: " << mixValue << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        mixValue -= 0.001f; // change this value accordingly (might be too slow or too fast based on system hardware)
        if (mixValue <= 0.0f)
            mixValue = 0.0f;
    }

    float cameraSpeed = 1.0f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->ProcessKeyboard(FORWARD, deltaFrameTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->ProcessKeyboard(BACKWARD, deltaFrameTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->ProcessKeyboard(LEFT, deltaFrameTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->ProcessKeyboard(RIGHT, deltaFrameTime);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->lookAt({0.f, 0.f, 0.f});
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
    app.camera->ProcessMouseMovement(xoffset, yoffset);
}

// mouse scroll callback
// ----------------------------------------------------------------------

void App::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    App& app = getInstance();
    app.camera->ProcessMouseScroll(yoffset);
}

void App::setupScene()
{
    scene_ = std::make_shared<Scene>();

    std::vector<float> cubePosArray(TestTriangle::CubeVertices, TestTriangle::CubeVertices + sizeof(TestTriangle::CubeVertices) / sizeof(float));
    std::vector<float> cubeUvArray(TestTriangle::CubeUVs, TestTriangle::CubeUVs + sizeof(TestTriangle::CubeUVs) / sizeof(float));
    std::vector<float> cubeNormalArray(TestTriangle::CubeNormals, TestTriangle::CubeNormals + sizeof(TestTriangle::CubeNormals) / sizeof(float));
    std::vector<unsigned> cubeIndexArray(TestTriangle::CubeIndices, TestTriangle::CubeIndices + sizeof(TestTriangle::CubeIndices) / sizeof(unsigned));
    BufferAttribute cubePosAttribute(cubePosArray, 3), cubeUvAttribute(cubeUvArray, 2), cubeNormalAttribute(cubeNormalArray, 3);

    auto cubeGeometry = std::make_shared<Geometry>();
    cubeGeometry->setAttribute("aPos", cubePosAttribute, true);
    cubeGeometry->setAttribute("aTexCoord", cubeUvAttribute);
    cubeGeometry->setAttribute("aNormal", cubeNormalAttribute);
    cubeGeometry->setIndex(cubeIndexArray);

    // lightMapCube
    //auto plightMapCubeMat = std::make_shared<LightMapMaterial>("material", 32.0f);
    //plightMapCubeMat->setpShader(Shader::loadShader("./assets/shader/MultiLight/multiLight.vert", "./assets/shader/MultiLight/multiLight.frag"));
    //plightMapCubeMat->loadDiffMap("./assets/texture/container2.png");
    //plightMapCubeMat->loadSpecMap("./assets/texture/container2_specular.png");
    //plightMapCubeMat->loadEmissiveMap("./assets/texture/matrix.jpg");
    //lightMapCube = std::make_shared<Object>(cubeGeometry, plightMapCubeMat);

    // nanosuit
    auto nanosuit = ResourceLoader::loadModel("./assets/models/nanosuit/nanosuit.obj");
    nanosuit->setScale(0.1, 0.1, 0.1);
    nanosuit->setPosition(glm::vec3(0.f, -0.5f, 0.f));
    nanosuit->setShadingMode(EShadingMode::Shading_BlinnPhong);
    nanosuit->enableCastShadow(true);
    scene_->addModel(nanosuit);

    // floor
    auto floor = ResourceLoader::loadModel("./assets/models/floor/floor.obj");
    floor->setPosition({0.f, -0.5f, 0.f});
    floor->setScale({0.1f, 0.1f, 0.1f});
    floor->setShadingMode(EShadingMode::Shading_BlinnPhong);
    floor->enableCastShadow(true);
    scene_->addModel(floor);


    // point light
    // shaders will be loaded by ShaderMode now, no longer needed to load manually.
    // here cube point light just use BaseColor.
    auto plightMaterial = std::make_shared<Material>();
    plightMaterial->setShadingMode(EShadingMode::Shading_BaseColor);

    {
        auto dirLight = ULight::makeLight(cubeGeometry, plightMaterial);
        glm::vec3 dirLightPos {1, 1, 1};
        dirLight->setAsDirectionalLight(nanosuit->getPosition() - dirLightPos, glm::vec3(0.1f), glm::vec3(0.5f), glm::vec3(0.5f));
        dirLight->setScale({ 0.1, 0.1, 0.1 });
        dirLight->setPosition(dirLightPos);
        scene_->addLight(dirLight);
    }

    // auto light2 = ULight::makeLight(cubeGeometry, plightMaterial);
    // light2->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f), 1.0, 0.045, 0.0075);
    // light2->setScale({ 0.1, 0.1, 0.1 });
    // light2->setPosition({1, 2, -1});
    // scene_->addLight(light2);

    {
        // auto dirLight2 = ULight::makeLight(cubeGeometry, plightMaterial);
        // glm::vec3 dirLightPos {1, 1, -1};
        // dirLight2->setAsDirectionalLight(floor->getPosition() - dirLightPos, glm::vec3(0.1f), glm::vec3(0.5f), glm::vec3(0.5f));
        // dirLight2->setScale({ 0.1, 0.1, 0.1 });
        // dirLight2->setPosition(dirLightPos);
        // scene_->addLight(dirLight2);
    }

    // spotlight
    //auto pspotlight = std::make_shared<Light>();
    ////auto pspotlight = std::make_shared<SpotLight>();
    //pspotlight->setAsSpotLight(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(25.5f)));
    ////pspotlight->setAttribute(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(25.5f)));
    //pspotlight->setName("spotLightArray");
    //lightArray.push_back(pspotlight);

    // camera pos
    camera->setPosition(0, 1, 1);
    GL_CHECK(;);
}

void App::beforeLoop() {
    GL_CHECK(glEnable(GL_DEPTH_TEST));
}

void App::draw()
{
    /*        glm::vec3 lightColor;
    lightColor.x = sin(glfwGetTime() * 2.0f);
    lightColor.y = sin(glfwGetTime() * 0.7f);
    lightColor.z = sin(glfwGetTime() * 1.3f);
    light->setColor(lightColor);  */

    // light
    //for (auto& light : lightArray) {
    //    light->draw(*camera);
    //    light->setColor(lightColor);
    //}


    //for (unsigned int i = 0; i < 10; i++)
    //{
    //    glm::mat4 model(1.0f);
    //    model = glm::translate(model, TestTriangle::cubePositions[i]);
    //    float angle = 20.0f * i;
    //    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    //    //pShader->setMat4("model", model);
    //    lightMapCube->setWorldMatrix(model);
    //    //cube->setLight(light); 
    //    //cube->draw(*camera);
    //    lightMapCube->setLightArray(lightArray);
    //    lightMapCube->draw(*camera);
    //    //glDrawArrays(GL_TRIANGLES, 0, 36
    //}

    //nanosuit->setLightArray(lightArray);
    //nanosuit->draw(*renderer);
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

void App::printMat4(const string& name, glm::mat4 mat) {
    std::cout << "Matrix manually:" + name + "\n";
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::cout << mat[i][j] << " ";
        }
        std::cout << std::endl;
    }
}
