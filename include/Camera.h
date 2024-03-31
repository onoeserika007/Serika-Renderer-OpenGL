#ifndef CAMERA_H
#define CAMERA_H

#include "Base/GLMInc.h"
#include "Utils//utils.h"

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

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
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
public:

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    virtual ~Camera() {}

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix() const 
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // returns the projection matrix calculated
    virtual glm::mat4 GetProjectionMatrix() const {
        //logDebug("Returing projection matix with fov" + std::to_string(fov_));
        return glm::mat4(1.0);
    }

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

    glm::vec3 position() const {
        return Position;
    }

    glm::vec3 forward() const {
        return Front;
    }

    void setPosition(float x, float y, float z) {
        Position = glm::vec3(x, y, z);
    }

    void setPosition(glm::vec3 pos) {
        Position = pos;
    }

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        // also re-calculate the Right and Up vector
        Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

class PerspectiveCamera :public Camera {
private:
    float fov_;
    float aspect_;
    float near_;
    float far_;
public:
    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        //logDebug("Camera processMouseScroll Called!");
        fov_ -= (float)yoffset;
        if (fov_ < 1.0f)
            fov_ = 1.0f;
        if (fov_ > 90.0f)
            fov_ = 90.0f;
    }

    virtual glm::mat4 GetProjectionMatrix() const override{
        //logDebug("Returing projection matix with fov" + std::to_string(fov_));
        return glm::perspective(glm::radians(fov_), aspect_, near_, far_);
    }

    void setAspect(float aspect) {
        aspect_ = aspect;
    }

    PerspectiveCamera(float fov = 45.0, float aspect = 1.0, float near = 0.1, float far = 1000.0, 
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), 
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), 
        float yaw = YAW, float pitch = PITCH)
    : fov_(fov), aspect_(aspect), near_(near), far_(far), Camera(position, up, yaw, pitch){}
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
    OrthographicCamera(float left = 1.0, float right = -1.0,
        float bottom = -1.0, float top = 1.0,
        float near = 0.1, float far = 1000.0,
        glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW, float pitch = PITCH) 
    : left_(left), right_(right), bottom_(bottom), top_(top) {}

    virtual glm::mat4 GetProjectionMatrix() const override {
        return glm::ortho(left_, right_, bottom_, top_);
    }
};

#endif