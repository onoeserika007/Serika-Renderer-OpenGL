#include "Application.h"

#include <iostream>

#include "Base/ProjectConfig.h"
#include "Utils/SRKLogger.h"

int main(){
    auto& config = ProjectConfig::getInstance();
    SerikaGL::SRKLogger::setLogLevel(SerikaGL::LogLevel::LOG_DEBUG);
    Application& app = Application::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();

    config.serialize(ProjectConfig::defaultConfigPath);
    return 0;
}
