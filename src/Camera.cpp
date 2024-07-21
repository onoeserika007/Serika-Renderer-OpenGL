#include "Camera.h"

#include "Base/Config.h"

Camera::Camera(glm::vec3 position, glm::vec3 up): Front(glm::vec3(0.0f, 0.0f, -1.0f)) {
    auto&& config = Config::getInstance();
    Position = position;
    WorldUp = up;
    Yaw = config.CameraYaw;
    Pitch = config.CameraPitch;
    MovementSpeed = config.CameraSpeed;
    MouseSensitivity = config.MouseSensitivity;
    Zoom = config.CameraZoom;
    updateCameraVectors();;;
};

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch): Camera({posX, posY, posZ}, {upX, upY, upZ}) {
}

void Camera::lookAt(const glm::vec3 &target) {
    const Rotator rotator {glm::normalize(target - position())};
    Yaw = rotator.Yaw;
    Pitch = rotator.Pitch;
    // glm::vec3 forward = glm::normalize(target - position());
    // float sa = glm::length((Rotator(Yaw, Pitch).toVec3() - forward));
    // std::cout << sa << std::endl;
    updateCameraVectors();
}

void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
};

void Camera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
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
};;

void Camera::ProcessMouseScroll(float yoffset) {
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::update() {
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // calculate the new Front vector
    const glm::vec3 front = Rotator(Yaw, Pitch).toVec3();
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

void PerspectiveCamera::ProcessMouseScroll(float yoffset) {
    //logDebug("Camera processMouseScroll Called!");
    fov_ -= (float)yoffset;
    if (fov_ < 1.0f)
        fov_ = 1.0f;
    if (fov_ > 90.0f)
        fov_ = 90.0f;
}

void PerspectiveCamera::update() {
    Camera::update(); // Super call

    // update frustum
    float nearHeightHalf = near_ * std::tan(fov_ / 2.f);
    float farHeightHalf = far_ * std::tan(fov_ / 2.f);
    float nearWidthHalf = nearHeightHalf * aspect_;
    float farWidthHalf = farHeightHalf * aspect_;

    // near plane
    glm::vec3 nearCenter = position() + forward() * near_;
    glm::vec3 nearNormal = forward();
    frustum_.planes[0].set(nearNormal, nearCenter);

    // far plane
    glm::vec3 farCenter = position() + forward() * far_;
    glm::vec3 farNormal = -forward();
    frustum_.planes[1].set(farNormal, farCenter);

    // top plane
    glm::vec3 topCenter = nearCenter + Up * nearHeightHalf;
    glm::vec3 topNormal = glm::cross(glm::normalize(topCenter - position()), Right);
    frustum_.planes[2].set(topNormal, topCenter);

    // bottom plane
    glm::vec3 bottomCenter = nearCenter - Up * nearHeightHalf;
    glm::vec3 bottomNormal = glm::cross(Right, glm::normalize(bottomCenter - position()));
    frustum_.planes[3].set(bottomNormal, bottomCenter);

    // left plane
    glm::vec3 leftCenter = nearCenter - Right * nearWidthHalf;
    glm::vec3 leftNormal = glm::cross(glm::normalize(leftCenter - position()), Up);
    frustum_.planes[4].set(leftNormal, leftCenter);

    // right plane
    glm::vec3 rightCenter = nearCenter + Right * nearWidthHalf;
    glm::vec3 rightNormal = glm::cross(Up, glm::normalize(rightCenter - position()));
    frustum_.planes[5].set(rightNormal, rightCenter);

    // 8 corners
    glm::vec3 nearTopLeft = nearCenter + Up * nearHeightHalf - Right * nearWidthHalf;
    glm::vec3 nearTopRight = nearCenter + Up * nearHeightHalf + Right * nearWidthHalf;
    glm::vec3 nearBottomLeft = nearCenter - Up * nearHeightHalf - Right * nearWidthHalf;
    glm::vec3 nearBottomRight = nearCenter - Up * nearHeightHalf + Right * nearWidthHalf;

    glm::vec3 farTopLeft = farCenter + Up * farHeightHalf - Right * farWidthHalf;
    glm::vec3 farTopRight = farCenter + Up * farHeightHalf + Right * farWidthHalf;
    glm::vec3 farBottomLeft = farCenter - Up * farHeightHalf - Right * farWidthHalf;
    glm::vec3 farBottomRight = farCenter - Up * farHeightHalf + Right * farWidthHalf;

    frustum_.corners[0] = nearTopLeft;
    frustum_.corners[1] = nearTopRight;
    frustum_.corners[2] = nearBottomLeft;
    frustum_.corners[3] = nearBottomRight;
    frustum_.corners[4] = farTopLeft;
    frustum_.corners[5] = farTopRight;
    frustum_.corners[6] = farBottomLeft;
    frustum_.corners[7] = farBottomRight;

    // bounding box
    frustum_.bbox.min_ = glm::vec3(std::numeric_limits<float>::max());
    frustum_.bbox.max_ = glm::vec3(std::numeric_limits<float>::min());
    for (auto &corner : frustum_.corners) {
        frustum_.bbox.min_.x = std::min(frustum_.bbox.min_.x, corner.x);
        frustum_.bbox.min_.y = std::min(frustum_.bbox.min_.y, corner.y);
        frustum_.bbox.min_.z = std::min(frustum_.bbox.min_.z, corner.z);

        frustum_.bbox.max_.x = std::max(frustum_.bbox.max_.x, corner.x);
        frustum_.bbox.max_.y = std::max(frustum_.bbox.max_.y, corner.y);
        frustum_.bbox.max_.z = std::max(frustum_.bbox.max_.z, corner.z);
    }

    frustum_center_ = (frustum_.bbox.min_ + frustum_.bbox.max_) / 2.f;
}
