#include "app.h"
#include "Base/Config.h"
#include "Utils/SRKLogger.h"

int main(){
    SerikaGL::SRKLogger::setLogLevel(SerikaGL::LogLevel::LOG_DEBUG);
    App& app = App::getInstance();
    app.init();
    app.mainLoop();
    app.terminate();

    auto&& config = Config::getInstance();
    config.serialize(Config::defaultConfigPath);
    return 0;
}
