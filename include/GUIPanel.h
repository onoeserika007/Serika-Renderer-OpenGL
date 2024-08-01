#pragma once
#include <functional>
#include <string>

class GUIPanel {
public:
    GUIPanel() = default;
    ~GUIPanel();
    bool init(void *window, int width, int height);
    void onDraw();

    void update();
    void updateSize(int width, int height);

    bool wantCaptureKeyboard();
    bool wantCaptureMouse();

    inline void setResetCameraFunc(const std::function<void(void)> &func) {
        resetCameraFunc_ = func;
    }

private:
    bool loadConfig();

    bool reloadModel(const std::string &name);
    bool reloadSkybox(const std::string &name);

    void drawSettings();
    void destroy();
private:
    int frameWidth_ = 0;
    int frameHeight_ = 0;

    std::function<void(void)> resetCameraFunc_;
};
