#include "OrbitController.h"
#include "FCamera.h"

OrbitController::OrbitController(FCamera &camera): camera_(camera) {
    INIT_EYE = camera.position();
    INIT_FOCUS = camera.position() + camera.forward();
    INIT_UP = camera.up();
    reset();
}

void OrbitController::update() {

    if (std::abs(zoomX) > motionEps || std::abs(zoomY) > motionEps) {
        zoomX /= motionSensitivity;
        zoomY /= motionSensitivity;
        zoomByPixels(zoomX, zoomY);
    } else {
        zoomX = 0;
        zoomY = 0;
    }

    if (std::abs(rotateX) > motionEps || std::abs(rotateY) > motionEps) {
        rotateX /= motionSensitivity;
        rotateY /= motionSensitivity;
        rotateByPixels(rotateX, rotateY);
    } else {
        rotateX = 0;
        rotateY = 0;
    }

    if (std::abs(panX) > motionEps || std::abs(panY) > motionEps) {
        panByPixels(panX, panY);
        panX = 0;
        panY = 0;
    }

    eye_ = center_ + armDir_ * armLength_;
    camera_.setPosition(eye_);
    camera_.lookAt(center_);
}

void OrbitController::recalculateCenter() {
    armDir_ = -camera_.forward();
    center_ = camera_.position() + camera_.forward() * armLength_;
}

void OrbitController::panByPixels(double dx, double dy) {
    glm::vec3 world_offset = camera_.getWorldPositionFromView(glm::vec3(dx, dy, 0));
    glm::vec3 world_origin = camera_.getWorldPositionFromView(glm::vec3(0));

    glm::vec3 delta = (world_origin - world_offset) * armLength_ * panSensitivity_;
    center_ += delta;
}

void OrbitController::rotateByPixels(double dx, double dy) {
    float x_angle = (float) dx * rotateSensitivity_;
    float y_angle = (float) dy * rotateSensitivity_;

    // turn eular to quat
    glm::qua<float> q = glm::qua<float>(glm::radians(glm::vec3(y_angle, -x_angle, 0)));
    glm::vec3 new_dir = glm::mat4_cast(q) * glm::vec4(armDir_, 1.0f);

    armDir_ = glm::normalize(new_dir);
}

void OrbitController::zoomByPixels(double dx, double dy) {
    armLength_ += -(float) dy * zoomSensitivity_;
    if (armLength_ < MIN_ORBIT_ARM_LENGTH) {
        armLength_ = MIN_ORBIT_ARM_LENGTH;
    }
    eye_ = center_ + armDir_ * armLength_;
}

void OrbitController::reset() {
    eye_ = INIT_EYE;
    center_ = INIT_FOCUS;
    up_ = INIT_UP;

    glm::vec3 dir = eye_ - center_;
    armDir_ = glm::normalize(dir);
    armLength_ = glm::length(dir);
}
