#include "Base/Config.h"

#include <fstream>

#include "json11/json11.hpp"
#include "Utils/Logger.h"

using json = json11::Json;;

const std::string Config::defaultConfigPath = "./configs/renderConfig.json";

void Config::serialize(const std::string &path) {
    json j = *this;
    std::ofstream ofs(path);
    try {
        if (!ofs) {
            throw std::ios_base::failure("Failed to open file for writing");
        }
        else {
            ofs << j.dump();// Pretty print with 4 spaces indentation
            ofs.close();
        }
    }
    catch (const std::system_error& e) {
        LOGE(e.what());
    }
}

void Config::deserialize(const std::string &path) {
    std::ifstream ifs(path);
    try {
        if (!ifs) {
            throw std::ios_base::failure("Failed to open file for reading");
        }
        else {
            const std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            ifs.close();;

            std::string err;
            json j = json::parse(str, err);
            if (!err.empty()) {
                std::cerr << "Error parsing JSON: " << err << std::endl;
            }

            from_json(j);
        }
    }
    catch (const std::system_error& e) {
        LOGI("No default config, creating default one...");
    }
}

Config & Config::getInstance() {
    static Config config;
    if (!config.loaded_) {

        std::lock_guard<std::mutex> guard(config.load_lock_);
        // prevent from loaded twice, already loaded other thread
        if (!config.loaded_) {
            config.deserialize(defaultConfigPath);
            std::cout << "Rendering Mode: " << static_cast<int>(config.RenderMode) << std::endl;
            config.loaded_ = true;
        }
    }
    return config;
}

json Config::to_json() const {
    return json::object {
        {"bShadowMap", json(bShadowMap) },
        {"RenderMode", json(static_cast<int>(RenderMode))},
        {"CameraYaw", json(CameraYaw)},
        {"CameraPitch", json(CameraPitch)},
        {"CameraSpeed", json(CameraSpeed)},
        {"MouseSensitivity", json(MouseSensitivity)},
        {"CameraZoom", json(CameraZoom)},
        {"CameraNear", json(CameraNear)},
        {"CameraFar", json(CameraFar)},
        {"Resolution_ShadowMap", json(Resolution_ShadowMap)},
        {"CaptureRadius_ShadowMap", json(CaptureRadius_ShadowMap)},
        {"CameraAspect", json(CameraAspect)},
        {"CameraFOV", json(CameraFOV)},
        {"WindowWidth", json(WindowWidth)},
        {"WindowHeight", json(WindowHeight)},
        {"bSkybox", json(bSkybox)},
        {"bUseBVH", json(bUseBVH)},
        {"SPP", json(SPP)},
        {"Exposure", json(Exposure)},
        {"bUseHDR", json(bUseHDR)},
        {"bUseBloom", json(bUseBloom)},
        {"bUseSSAO", json(bUseSSAO)},
        {"SceneType", json(SceneType)}
    };;;
}

Config & Config::from_json(const json &j) {
    bShadowMap = j["bShadowMap"].bool_value();
    RenderMode = static_cast<ERenderMode>(j["RenderMode"].int_value());
    CameraYaw = j["CameraYaw"].number_value();
    CameraPitch = j["CameraPitch"].number_value();
    CameraSpeed = j["CameraSpeed"].number_value();
    MouseSensitivity = j["MouseSensitivity"].number_value();
    CameraZoom = j["CameraZoom"].number_value();;
    CameraNear = j["CameraNear"].number_value();
    CameraFar = j["CameraFar"].number_value();
    Resolution_ShadowMap = j["Resolution_ShadowMap"].int_value();
    CaptureRadius_ShadowMap = j["CaptureRadius_ShadowMap"].number_value();
    CameraAspect = j["CameraAspect"].number_value();
    CameraFOV = j["CameraFOV"].number_value();
    WindowWidth = j["WindowWidth"].int_value();
    WindowHeight = j["WindowHeight"].int_value();
    bSkybox = j["bSkybox"].bool_value();
    bUseBVH = j["bUseBVH"].bool_value();
    SPP = j["SPP"].int_value();
    bUseHDR = j["bUseHDR"].bool_value();
    Exposure = j["Exposure"].number_value();
    bUseBloom = j["bUseBloom"].bool_value();
    bUseSSAO = j["bUseSSAO"].bool_value();
    SceneType = static_cast<ESceneType>(j["SceneType"].int_value());
    return *this;;;;;;
}

