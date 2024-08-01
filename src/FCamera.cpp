#include "FCamera.h"

#include "Base/Config.h";

FCamera::FCamera(glm::vec3 position, glm::vec3 up)
: Front(glm::vec3(0.0f, 0.0f, -1.0f)) {
    auto&& config = Config::getInstance();
    Position = position;
    WorldUp = up;
    Yaw = config.CameraYaw;
    Pitch = config.CameraPitch;
    MovementSpeed = config.CameraSpeed;
    MouseSensitivity = config.MouseSensitivity;
    Zoom = config.CameraZoom;
    updateCameraVectors();;
};

FCamera::FCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch): FCamera({posX, posY, posZ}, {upX, upY, upZ}) {
}

void FCamera::lookAt(const glm::vec3 &target) {
    // target和eye不能重合！！否则会出现除0错误
    // 在normalize这一步就已经Nan了
    glm::vec3 direction = target - position();
    if (glm::length(direction) < M_EPSILON) direction = glm::vec3(0.f, 0.f, -1.f);
    const Rotator rotator {glm::normalize(direction)};
    Yaw = rotator.Yaw;;
    Pitch = rotator.Pitch;

    glm::vec3 forward = glm::normalize(target - position());

    glm::mat4 rotation = glm::lookAt(position(), target - position(), Up);
    glm::vec4 fVector = glm::inverse(rotation) * glm::vec4({0.f, 0.f, -1.f, 0.f});
    glm::vec3 res = glm::normalize(glm::vec3(fVector.x , fVector.y , fVector.z));

    // float sa = glm::length((res - forward));
    // printVec3("1: ", forward);
    // printVec3("2: ", res);
    // std::cout << sa << std::endl;

    updateCameraVectors();;
}

void FCamera::ProcessKeyboard(Camera_Movement direction, float deltaTime) {
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

void FCamera::ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch) {
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

void FCamera::ProcessMouseScroll(const float yoffset) {
    Zoom -= yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void FCamera::debugCamera() const {
    printVec3("Position: ", position());
    printVec3("Front: ", Front);
    printVec3("Up: ", Up);
    printVec3("Right: ", Right);
    printVec3("WorldUp: ", WorldUp);
    std::cout << "Yaw: " << Yaw << std::endl;
    std::cout << "Pitch: " << Pitch << std::endl;
    std::cout << "MovementSpeed: " << MovementSpeed << std::endl;
    std::cout << "MouseSensitivity: " << MouseSensitivity << std::endl;
    std::cout << "Zoom: " << Zoom << std::endl;
}

void FCamera::update() {
    updateCameraVectors();
}

void FCamera::updateCameraVectors() {
    // calculate the new Front vector
    const glm::vec3 front = Rotator(Yaw, Pitch).toVec3();
    Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    Up = glm::normalize(glm::cross(Right, Front));
}

PerspectiveCamera::PerspectiveCamera(float fov, float aspect, float near, float far, glm::vec3 position, glm::vec3 up): FCamera(position, up), fov_(fov), aspect_(aspect) {
    // auto proj = glm::perspective(glm::radians(fov_), aspect_, near_, far_);
    // std::this_thread::sleep_for(std::chrono::microseconds(1));
    near_ = near;
    far_ = far;
}

void PerspectiveCamera::ProcessMouseScroll(const float yoffset) {
    fov_ -= yoffset;
    if (fov_ < 1.0f)
        fov_ = 1.0f;
    if (fov_ > 90.0f)
        fov_ = 90.0f;
}

void PerspectiveCamera::debugCamera() const {
    FCamera::debugCamera();
    std::cout << "FOV: " << fov_ << std::endl;
    std::cout << "Aspect: " << aspect_ << std::endl;
    std::cout << "Near: " << near_ << std::endl;
    std::cout << "Fov: " << far_ << std::endl;
    printMat4("Perspective Projection: ", GetProjectionMatrix());
    printMat4("Perspective Projection(directly): ", glm::perspective(glm::radians(fov_), aspect_, near_, far_));
}

glm::mat4 PerspectiveCamera::GetProjectionMatrix() const {
    return glm::perspective(glm::radians(fov_), aspect_, near_, far_);
}

void PerspectiveCamera::update() {
    FCamera::update(); // Super call

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
