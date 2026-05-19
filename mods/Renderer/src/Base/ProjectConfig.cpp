#include "Base/ProjectConfig.h"

#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include "Utils/SRKLogger.h"

using json = nlohmann::json;

const std::string ProjectConfig::defaultConfigPath = "./configs/renderConfig.json";

void ProjectConfig::serialize(const std::string &path) {
    json j = to_json();
    std::ofstream ofs(path);
    try {
        if (!ofs) {
            throw std::ios_base::failure("Failed to open file for writing");
        }
        ofs << j.dump(4);
        ofs.close();
    }
    catch (const std::system_error& e) {
        LOGE(e.what());
    }
}

void ProjectConfig::deserialize(const std::string &path) {
    std::ifstream ifs(path);
    try {
        if (!ifs) {
            throw std::ios_base::failure("Failed to open file for reading");
        }
        json j = json::parse(ifs);
        ifs.close();
        from_json(j);
    }
    catch (const json::parse_error& e) {
        LOGE(e.what());
    }
    catch (const std::system_error& e) {
        LOGI("No default config, using defaults...");
    }
}

ProjectConfig & ProjectConfig::getInstance() {
    static ProjectConfig config;
    if (!config.loaded_) {
        std::lock_guard<std::mutex> guard(config.load_lock_);
        if (!config.loaded_) {
            config.deserialize(defaultConfigPath);
            std::cout << "Rendering RenderPipeline: " << static_cast<int>(config.RenderPipeline) << std::endl;
            config.loaded_ = true;
        }
    }
    return config;
}

json ProjectConfig::to_json() const {
    return {
        {"bShadowMap", bShadowMap},
        {"RenderPipeline", static_cast<int>(RenderPipeline)},
        {"CameraYaw", CameraYaw},
        {"CameraPitch", CameraPitch},
        {"CameraSpeed", CameraSpeed},
        {"MouseSensitivity", MouseSensitivity},
        {"CameraZoom", CameraZoom},
        {"CameraNear", CameraNear},
        {"CameraFar", CameraFar},
        {"Resolution_ShadowMap", Resolution_ShadowMap},
        {"CaptureRadius_ShadowMap", CaptureRadius_ShadowMap},
        {"CameraAspect", CameraAspect},
        {"CameraFOV", CameraFOV},
        {"WindowWidth", WindowWidth},
        {"WindowHeight", WindowHeight},
        {"bSkybox", bSkybox},
        {"bUseBVH", bUseBVH},
        {"SPP", SPP},
        {"Exposure", Exposure},
        {"bUseHDR", bUseHDR},
        {"bUseBloom", bUseBloom},
        {"bUseSSAO", bUseSSAO},
        {"SceneType", static_cast<int>(SceneType)},
        {"ShadingModelForDeferredRendering", static_cast<int>(ShadingModelForDeferredRendering)},
        {"bUseMipmaps", bUseMipmaps},
        {"bDrawDebugBVH", bDrawDebugBVH}
    };
}

ProjectConfig & ProjectConfig::from_json(const json &j) {
    bShadowMap = j["bShadowMap"].get<bool>();
    RenderPipeline = static_cast<ERenderPipeline>(j["RenderPipeline"].get<int>());
    CameraYaw = j["CameraYaw"].get<float>();
    CameraPitch = j["CameraPitch"].get<float>();
    CameraSpeed = j["CameraSpeed"].get<float>();
    MouseSensitivity = j["MouseSensitivity"].get<float>();
    CameraZoom = j["CameraZoom"].get<float>();
    CameraNear = j["CameraNear"].get<float>();
    CameraFar = j["CameraFar"].get<float>();
    Resolution_ShadowMap = j["Resolution_ShadowMap"].get<int>();
    CaptureRadius_ShadowMap = j["CaptureRadius_ShadowMap"].get<float>();
    CameraAspect = j["CameraAspect"].get<float>();
    CameraFOV = j["CameraFOV"].get<float>();
    WindowWidth = j["WindowWidth"].get<int>();
    WindowHeight = j["WindowHeight"].get<int>();
    bSkybox = j["bSkybox"].get<bool>();
    bUseBVH = j["bUseBVH"].get<bool>();
    SPP = j["SPP"].get<int>();
    bUseHDR = j["bUseHDR"].get<bool>();
    Exposure = j["Exposure"].get<float>();
    bUseBloom = j["bUseBloom"].get<bool>();
    bUseSSAO = j["bUseSSAO"].get<bool>();
    SceneType = static_cast<ESceneType>(j["SceneType"].get<int>());
    ShadingModelForDeferredRendering = static_cast<EShadingModel>(j["ShadingModelForDeferredRendering"].get<int>());
    bUseMipmaps = j["bUseMipmaps"].get<bool>();
    bDrawDebugBVH = j["bDrawDebugBVH"].get<bool>();
    return *this;
}
