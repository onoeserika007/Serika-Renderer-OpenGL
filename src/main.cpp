// learnOpenGL-aTriangle.cpp: 定义应用程序的入口点。
#include "Base/GLMInc.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "primitives.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <memory>
#include "Utils/utils.h"
#include "Shader.h"
#include "ShaderGLSL.h"
#include "Camera.h"
#include "Texture.h"
#include <functional>
#include "Geometry.h"
#include "BufferAttribute.h"
#include "Object.h"
#include "Material.h"
#include "Light.h"
#include "Model.h"
#include "RendererOpenGL.h"
#include "ViewerOpenGL.h"
#include "Utils/OpenGLUtils.h"
#include "Base/Config.h"
#include "Scene.h"

using namespace std;

class App
{
    float mixValue = 0.5;
    static bool firstMouse;

    int windowWidth = 0;
    int windowHeight = 0;
    std::shared_ptr<Object> cube;
    std::shared_ptr<Object> lightMapCube;
    std::shared_ptr<Light> light;
    std::vector<std::shared_ptr<Light>> lightArray;
    static std::shared_ptr<PerspectiveCamera> camera;
    static std::shared_ptr<Viewer> viewer_;
    //std::shared_ptr<Renderer> renderer;
    std::shared_ptr<Scene> scene_;
    GLFWwindow* window = nullptr;
    float lastFrameTime = 0.0;
    float deltaFrameTime = 0.01;

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    static float lastX;
    static float lastY;
    App() : model(1.0f), view(1.0f), projection(1.0f) {}
public:
    static App& getInstance() {
        static App app;
        return app;
    }

    void init() {
        setupWindow();
        setupResource();
        printMaxVertexAttributeNum();
        beforeLoop();
    }

    void mainLoop() {
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

    void setupWindow() {
        glfwInit();
        //glGetError();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);                 // OpenGL 主版本号
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);                 // OpenGL 次版本号
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 使用核心模式
        // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
        if (window == NULL)
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

        // render config
        Config config;

        // viewer
        viewer_ = std::make_shared<ViewerOpenGL>(*camera, config);
        viewer_->init(frameWidth, frameHeight, 0);
        //viewer_ = nullptr;
        //ViewerOpenGL testViewer{ *camera, config };

        auto local_framebuffer_size_callback = [this](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
            viewer_->setViewPort(0, 0, width, height);
            camera->setAspect(static_cast<float>(width) / height);
            };

        // called when resized
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        if (viewer_) {
            viewer_->setViewPort(0, 0, width, height);
        }
        else {
            glViewport(0, 0, width, height);
        }
        camera->setAspect(static_cast<float>(width) / height);
    }

    void processInput()
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
    }

    // mouse callback
    // ----------------------------------------------------------------------
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        if (firstMouse) // 这个bool变量初始时是设定为true的
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // 注意这里是相反的，因为y坐标是从底部往顶部依次增大的
        lastX = xpos;
        lastY = ypos;
        camera->ProcessMouseMovement(xoffset, yoffset);
    }

    // mouse scroll callback
    // ----------------------------------------------------------------------
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera->ProcessMouseScroll(yoffset);
    }

    void setupResource()
    {
        scene_ = std::make_shared<Scene>();


        std::vector<float> cubePosArray, cubeUvArray, cubeNormalArray;
        for (int i = 0; i < sizeof(TestTriangle::cubeVertices) / sizeof(float); i += 5) {
            int j = 0;
            for (; j < 3; j++) {
                cubePosArray.push_back(TestTriangle::cubeVertices[i + j]);
            }
            for (; j < 5; j++) {
                cubeUvArray.push_back(TestTriangle::cubeVertices[i + j]);
            }
        }

        for (int i = 0; i < sizeof(TestTriangle::cubeVerticesWithNormals) / sizeof(float); i += 6) {
            int j = 0;
            while (j < 3) j++;
            while (j < 6) {
                cubeNormalArray.push_back(TestTriangle::cubeVerticesWithNormals[i + j]);
                j++;
            }
        }
        BufferAttribute cubePosAttribute(cubePosArray, 3), cubeUvAttribute(cubeUvArray, 2), cubeNormalAttribute(cubeNormalArray, 3);


        //auto cubeGeometry = std::make_shared<Geometry>();
        auto cubeGeometry = std::make_shared<Geometry>();
        cubeGeometry->setAttribute("aPos", cubePosAttribute, true);
        cubeGeometry->setAttribute("aTexCoord", cubeUvAttribute);
        cubeGeometry->setAttribute("aNormal", cubeNormalAttribute);


        // Shader
        //auto pShader = std::make_shared<Shader>("./assets/shader/vertexShader.vert", "./assets/shader/fragmentShader.frag");
        //auto pShader = Shader::loadShader("./assets/shader/vertexShader.vert", "./assets/shader/fragmentShader.frag");
        // 我们还要通过使用glUniform1i设置每个采样器的方式告诉OpenGL每个着色器采样器属于哪个纹理单元。我们只需要设置一次即可，所以这个会放在渲染循环的前面：
        //glUniform1i(glGetUniformLocation(pShader->ID, "texture1"), 0); // 手动设置
        //pShader->setInt("texture2", 1); // 或者使用着色器类设置

        // cube
        //auto pcubeMaterial = std::make_shared<PhongMaterial>("material", glm::vec3(0.0, 0.1, 0.06), glm::vec3(0.0, 0.50980392, 0.50980392), glm::vec3(0.50196078, 0.50196078, 0.50196078), 32.0);
        //cube = std::make_shared<Object>(cubeGeometry, pcubeMaterial);
        //cube = std::make_shared<Object<float>>(cubeGeometry, plightedObjectShader);

        // lightMapCube
        //auto plightMapCubeMat = std::make_shared<LightMapMaterial>("material", 32.0f);
        //plightMapCubeMat->setpShader(Shader::loadShader("./assets/shader/MultiLight/multiLight.vert", "./assets/shader/MultiLight/multiLight.frag"));
        //plightMapCubeMat->loadDiffMap("./assets/texture/container2.png");
        //plightMapCubeMat->loadSpecMap("./assets/texture/container2_specular.png");
        //plightMapCubeMat->loadEmissiveMap("./assets/texture/matrix.jpg");
        //lightMapCube = std::make_shared<Object>(cubeGeometry, plightMapCubeMat);

        // nanosuit
        std::string nanosuitPath = "./assets/models/nanosuit/nanosuit.obj";
        auto nanosuit = std::make_shared<Model>(nanosuitPath);
        nanosuit->setScale(0.1, 0.1, 0.1);
        GL_CHECK(;);

        scene_->addModel(nanosuit);
        GL_CHECK(;);
        //auto nanoShader = renderer->createShader("./assets/shader/nanosuit/nanosuit.vert", "./assets/shader/nanosuit/nanosuit.frag");
        // 模型不同mesh之间的shader不应该共享
        //auto nanoShader = renderer->createShader("./assets/shader/Standard.vert", "./assets/shader/Standard.frag");
        ////auto nanoShader = Shader::loadShader("./assets/shader/nanosuit/nanosuit.vert", "./assets/shader/nanosuit/nanosuit.frag");
        //nanosuit->setShader(nanoShader);

        // point light
        //auto plightShader = Shader::loadShader("./assets/shader/light.vert", "./assets/shader/light.frag");
        //auto plightMaterial = std::make_shared<Material>("lightSurface", plightShader);
        //light = std::make_shared<Light>("light", glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), cubeGeometry, plightMaterial);
        ////auto light = std::make_shared<PointLight>(cubeGeometry, plightMaterial);
        //light->setAsPointLight(glm::vec3(0, 0, 0), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.045, 0.0075);
        ////light->setAttribute(glm::vec3(0, 0, 0), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.045, 0.0075);
        //light->setName("pointLightArray");
        //glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, lightPos);
        //model = glm::scale(model, glm::vec3(0.2f));
        //auto lightShader = light->getpShader();
        //light->setWorldMatrix(model);
        //lightArray.push_back(light);

        // spotlight
        //auto pspotlight = std::make_shared<Light>();
        ////auto pspotlight = std::make_shared<SpotLight>();
        //pspotlight->setAsSpotLight(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(25.5f)));
        ////pspotlight->setAttribute(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(25.5f)));
        //pspotlight->setName("spotLightArray");
        //lightArray.push_back(pspotlight);

        // camera pos
        camera->setPosition(0, 0, 5);
        GL_CHECK(;);
    }

    void beforeLoop() {
        GL_CHECK(glEnable(GL_DEPTH_TEST));

        //nanosuit->setupPipeline(*renderer);
    }

    void draw()
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
        viewer_->drawScene(scene_);
    }

    static void printMaxVertexAttributeNum() {
        int nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
    }

    void terminate() {
        // 先析构Renderer中的UniformBlock 但是有8个引用，资源肯定是泄露了
        // 不是基类虚函数的问题，智能指针可以解决这个问题
        // 是object也和app一个生命周期，app直到进程退出才被销毁
        // renderer = nullptr;
        // 释放资源
        scene_ = nullptr;
        viewer_ = nullptr;
        glfwTerminate();
    }

    void printMat4(const string& name, glm::mat4 mat) {
        std::cout << "Matrix manually:" + name + "\n";
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                std::cout << mat[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }

};

float App::lastX = 400;
float App::lastY = 300;
bool App::firstMouse = false;
std::shared_ptr<PerspectiveCamera> App::camera(nullptr);
std::shared_ptr<Viewer> App::viewer_(nullptr);

int main(){
    SerikaGL::Logger::setLogLevel(SerikaGL::LogLevel::LOG_DEBUG);
    App app = App::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();
    return 0;
}


