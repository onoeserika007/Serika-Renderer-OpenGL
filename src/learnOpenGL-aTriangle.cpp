// learnOpenGL-aTriangle.cpp: 定义应用程序的入口点。
#include "learnOpenGL-aTriangle.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "primitives.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <memory>
#include "Utils/utils.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <functional>
#include "Geometry.h"
#include "BufferAttribute.h"
using namespace std;

class App
{
    //unsigned int VBO = 0;
    //unsigned int VAO = 0;
    //unsigned int EBO = 0;
    
    unsigned int shaderProgram = 0;
    float mixValue = 0.5;
    static bool firstMouse;

    int windowWidth = 0;
    int windowHeight = 0;
    std::shared_ptr<Shader> pShader;
    Texture wallTexture;
    Texture faceTexture;
    std::shared_ptr<Geometry> cube;
    static std::shared_ptr<PerspectiveCamera> camera;
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
        while (!glfwWindowShouldClose(window))
        {
            float currentFrameTime = glfwGetTime();
            deltaFrameTime = currentFrameTime - lastFrameTime;
            lastFrameTime = currentFrameTime;
            processInput();

            // render
            // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            //glClear(GL_COLOR_BUFFER_BIT); // GL_COLOR_BUFFER_BIT, GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            draw();

            glfwSwapBuffers(window); // 交换(Swap)前缓冲和后缓冲
            glfwPollEvents();        // 检测事件
        }

        glfwTerminate();
    }

    void setupWindow() {
        glfwInit();
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
        glViewport(0, 0, 800, 600);

        // called when resized
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // camera
        //camera = std::make_shared<Camera>(0, 0, 5);
        camera = std::make_shared<PerspectiveCamera>();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetScrollCallback(window, scroll_callback);
    }

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

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        camera->ProcessMouseScroll(yoffset);
    }

    void setupResource()
    {
        //// Vertex Array Object
        //// 等于说VAO也是一种上下文，在设置VBO前绑定，解绑后，需要绘制时再绑定
        //glGenVertexArrays(1, &VAO);
        //glBindVertexArray(VAO);

        //// VBO存放在GPU内存中，即显存
        //// 生成缓冲数量 
        //glGenBuffers(1, &VBO);
        //// OpenGL允许我们同时绑定多个缓冲，只要它们是不同的缓冲类型
        //glBindBuffer(GL_ARRAY_BUFFER, VBO);
        //// 把之前定义的顶点数据复制到缓冲的内存中
        //// 我们希望显卡如何管理给定的数据
        //// GL_STATIC_DRAW ：数据不会或几乎不会改变。
        //// GL_DYNAMIC_DRAW：数据会被改变很多。
        //// GL_STREAM_DRAW ：数据每次绘制时都会改变。
        //// 比如说一个缓冲中的数据将频繁被改变，那么使用的类型就是GL_DYNAMIC_DRAW或GL_STREAM_DRAW，这样就能确保显卡把数据放在能够高速写入的内存部分。
        ////glBufferData(GL_ARRAY_BUFFER, sizeof(TestTriangle::vertices), TestTriangle::vertices, GL_STATIC_DRAW);
        ////glBufferData(GL_ARRAY_BUFFER, sizeof(TestTriangle::verticesWithIndex), TestTriangle::verticesWithIndex, GL_STATIC_DRAW);
        ////glBufferData(GL_ARRAY_BUFFER, sizeof(TestTriangle::verticesWithIndexAndColor), TestTriangle::verticesWithIndexAndColor, GL_STATIC_DRAW);
        ////glBufferData(GL_ARRAY_BUFFER, sizeof(TestTriangle::verticesWithTex), TestTriangle::verticesWithTex, GL_STATIC_DRAW);
        //glBufferData(GL_ARRAY_BUFFER, sizeof(TestTriangle::cubeVertices), TestTriangle::cubeVertices, GL_STATIC_DRAW);

        //// EBO
        ////glGenBuffers(1, &EBO);
        ////glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        ////glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TestTriangle::indices), TestTriangle::indices, GL_STATIC_DRAW);

        //// 指定顶点属性的解释方式（如何解释VBO中的数据）
        //// 1. glVertexAttribPointer
        //// attri的Location(layout location = 0) | 属性大小 | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
        ////glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        ////glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        ////glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        ////// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
        ////glEnableVertexAttribArray(0);
        ////glEnableVertexAttribArray(1);
        ////glEnableVertexAttribArray(2);

        //// for cube
        //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        //glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        //glEnableVertexAttribArray(0);
        //glEnableVertexAttribArray(1);
        std::vector<float> cubePosArray, cubeUvArray;
        for (int i = 0; i < sizeof(TestTriangle::cubeVertices) / sizeof(float); i += 5) {
            int j = 0;
            for (; j < 3; j++) {
                cubePosArray.push_back(TestTriangle::cubeVertices[i + j]);
            }
            for (; j < 5; j++) {
                cubeUvArray.push_back(TestTriangle::cubeVertices[i + j]);
            }
        }
        BufferAttribute<float> cubePosAttribute(cubePosArray, 3), cubeUvAttribute(cubeUvArray, 2);


        cube = std::make_shared<Geometry>();
        cube->setAttribute("position", cubePosAttribute, 0);
        cube->setAttribute("uv", cubeUvAttribute, 1);

        // Shader
        pShader = std::make_shared<Shader>("./assets/shader/vertexShader.vert", "./assets/shader/fragmentShader.frag");

        // Texture
        wallTexture = Texture::loadTexture("./assets/texture/wall.jpg");
        faceTexture = Texture::loadTexture("./assets/texture/awesomeface.png");
    }

    void beforeLoop() {
        pShader->use(); // 不要忘记在设置uniform变量之前激活着色器程序！
        // 我们还要通过使用glUniform1i设置每个采样器的方式告诉OpenGL每个着色器采样器属于哪个纹理单元。我们只需要设置一次即可，所以这个会放在渲染循环的前面：
        glUniform1i(glGetUniformLocation(pShader->ID, "texture1"), 0); // 手动设置
        pShader->setInt("texture2", 1); // 或者使用着色器类设置
        glEnable(GL_DEPTH_TEST);
    }

    void draw()
    {
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // GL_FILL
        //float timeValue = glfwGetTime();
        //float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
        //int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor"); // -1 for not found
        setMVP();
        pShader->setMat4("model", model);
        //std::cout <<  << std::endl;
        //printMat4("model", model);
        pShader->setMat4("view", view);
        pShader->setMat4("projection", projection);
        wallTexture.use();
        faceTexture.use();
        pShader->setFloat("mixValue", mixValue);
        pShader->setMat4("transform", generateTransform());
        //glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, TestTriangle::cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            pShader->setMat4("model", model);

            cube->draw();
            //glDrawArrays(GL_TRIANGLES, 0, 36
        }
        //cube->draw();
        //glDrawArrays(GL_TRIANGLES, 0, 36); // primitive | 顶点数组其实索引 | 绘制数量
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // primitive | nums | 索引类型 | 最后一个参数里我们可以指定EBO中的偏移量（或者传递一个索引数组，但是这是当你不在使用EBO的时候），但是我们会在这里填写0。
    }

    static void printMaxVertexAttributeNum() {
        int nrAttributes;
        glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
        std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes << std::endl;
    }

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
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

    void terminate() {
        // 释放资源
        glfwTerminate();
    }

    glm::mat4 generateTransform() {
        glm::mat4 trans(1.0f);
        //trans = glm::rotate(trans, glm::radians(90.0f), glm::vec3(0.0, 0.0, 1.0));
        trans = glm::rotate(trans, glm::radians((float)glfwGetTime()), glm::vec3(0.0, 0.0, 1.0));
        trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
        return trans;
    }

    void setMVP() {
        model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // 因为在view变换中，默认最终相机是在原点看向z负轴的，所以这里是向z轴正方向移动了三个单位
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));
        // view 
        //float radius = 10.0f;
        //float camX = sin(glfwGetTime()) * radius;
        //float camZ = cos(glfwGetTime()) * radius;
        //view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
        //view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        view = camera->GetViewMatrix();
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        projection = camera->GetProjectionMatrix();
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

int main(){
    App app = App::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();
    return 0;
}


