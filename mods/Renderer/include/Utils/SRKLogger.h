#ifndef SERIKA_RENDERER_INCLUDE_UTILS_SRKLOGGER_H
#define SERIKA_RENDERER_INCLUDE_UTILS_SRKLOGGER_H
#include <cstdarg>
#include <mutex>

namespace SerikaGL {

#define LOGI(...) SerikaGL::SRKLogger::log(SerikaGL::LOG_INFO,     __FILE__, __LINE__, __VA_ARGS__)
#define LOGD(...) SerikaGL::SRKLogger::log(SerikaGL::LOG_DEBUG,    __FILE__, __LINE__, __VA_ARGS__)
#define LOGW(...) SerikaGL::SRKLogger::log(SerikaGL::LOG_WARNING,  __FILE__, __LINE__, __VA_ARGS__)
#define LOGE(...) SerikaGL::SRKLogger::log(SerikaGL::LOG_ERROR,    __FILE__, __LINE__, __VA_ARGS__)

	static constexpr int MAX_LOG_LENGTH = 1024;

	typedef void (*LogFunc)(void* context, int level, const char* msg);

	enum LogLevel {
		LOG_INFO,
		LOG_DEBUG,
		LOG_WARNING,
		LOG_ERROR,
	};

	class SRKLogger {
	public:
		static void setLogFunc(void* ctx, LogFunc func);
		static void setLogLevel(LogLevel level);
		static void log(LogLevel level, const char* file, int line, const char* message, ...);

	private:
		static void* logContext_;
		static LogFunc logFunc_;
		static LogLevel minLevel_;

		static char buf_[MAX_LOG_LENGTH];
		static std::mutex mutex_;
	};

	void printStackTrace();
	void protectedCall(void* context, int level, const char* file, int line, const char* message);

}

#endif // SERIKA_RENDERER_INCLUDE_UTILS_SRKLOGGER_H
