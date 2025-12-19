#include "Application.h"
#include "Base/Config.h"
#include "Utils/SRKLogger.h"

int main(){
    auto& config = Config::getInstance();
    SerikaGL::SRKLogger::setLogLevel(SerikaGL::LogLevel::LOG_DEBUG);
    Application& app = Application::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();

    config.serialize(Config::defaultConfigPath);
    return 0;
}
