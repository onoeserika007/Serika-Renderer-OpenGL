#ifndef CAMERA_H
#define CAMERA_H

#include "Base/GLMInc.h"
#include "Utils//utils.h"
#include "Geometry/Geometry.h"

enum class CameraType {
    ORTHOGRAPHIC,
    PERSPECTIVE,
};

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

struct Rotator {
    float Yaw; // degrees
    float Pitch;
    float Roll;

    glm::vec3 toVec3() const {
        // 计算时要注意虚幻和opengl的坐标系是不一样的，虚幻是z轴向上
        glm::vec3 ret;
        ret.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        ret.y = sin(glm::radians(Pitch));
        ret.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        return glm::normalize(ret);
    }

    Rotator() = default;
    Rotator(float inYaw, float inPitch, float inRoll = 0): Yaw(inYaw), Pitch(inPitch), Roll(inRoll) {}
    explicit Rotator(const glm::vec3& inVector)  {
        Yaw = glm::degrees(atan2(inVector.z, inVector.x));
        Pitch = glm::degrees(atan2(inVector.y, sqrt(inVector.x * inVector.x + inVector.z * inVector.z)));
        Roll = 0.f;
    }
};


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f));

    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

    virtual ~Camera() {}

    void setPosition(float x, float y, float z) { Position = glm::vec3(x, y, z);}

    void setPosition(glm::vec3 pos) { Position = pos; }

    void lookAt(const glm::vec3& target);

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front * 10.f, Up); }

    // returns the projection matrix calculated
    virtual glm::mat4 GetProjectionMatrix() const { return glm::mat4(1.0); }

    glm::vec3 position() const { return Position; }

    glm::vec3 forward() const { return Front; }

    glm::vec3 getFrustumCenter() const { return frustum_center_; }

protected:

    virtual void update();

    // camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    // euler Angles
    float Yaw;
    float Pitch;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Frustum frustum_;
    glm::vec3 frustum_center_ {};

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors();
};

class PerspectiveCamera :public Camera {
public:
    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset);

    virtual glm::mat4 GetProjectionMatrix() const override{
        return glm::perspective(glm::radians(fov_), aspect_, near_, far_);
    }

    void setAspect(float aspect) {
        aspect_ = aspect;
    }

    PerspectiveCamera(float fov = 45.0, float aspect = 1.0, float near = 0.1, float far = 1000.0,
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
    : fov_(fov), aspect_(aspect), near_(near), far_(far), Camera(position, up){}
protected:
    virtual void update() override;

private:
    float fov_;
    float aspect_;
    float near_;
    float far_;
};

class OrthographicCamera : public Camera {
private:
    float left_;
    float right_;
    float bottom_;
    float top_;
    float near_;
    float far_;
public:
    OrthographicCamera(float left = -1.0f, float right = 1.0f,
        float bottom = -1.0f, float top = 1.0f,
        float near = 0.1f, float far = 10.f,
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
    : left_(left), right_(right), bottom_(bottom), top_(top), near_(near), far_(far), Camera(position, up) {}

    virtual glm::mat4 GetProjectionMatrix() const override {
        return glm::ortho(left_, right_, bottom_, top_, near_, far_);
    }
};

#endif