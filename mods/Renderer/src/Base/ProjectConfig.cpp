#include "Base/ProjectConfig.h"

#include <iostream>

#include "Utils/SRKLogger.h"

const std::string ProjectConfig::defaultConfigPath = "./configs/renderConfig.json";

void ProjectConfig::serialize(const std::string &path) {
    const ProjectConfigData &data = *this;
    if (!Serika::writeJsonFile(path, data)) {
        LOGE("Failed to write project config: %s", path.c_str());
    } else {
        LOGI("Project config saved: %s", path.c_str());
    }
}

void ProjectConfig::deserialize(const std::string &path) {
    ProjectConfigData data = *this;
    if (Serika::readJsonFile(path, data)) {
        static_cast<ProjectConfigData &>(*this) = data;
        LOGI("Project config loaded: %s", path.c_str());
    } else {
        LOGI("Project config unavailable, using defaults: %s", path.c_str());
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
