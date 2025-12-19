#ifndef SERIKA_RENDERER_INCLUDE_UTILS_OPENGLUTILS_H
#define SERIKA_RENDERER_INCLUDE_UTILS_OPENGLUTILS_H
void checkGLError_(const char* stmt, const char* file, int line);

#ifdef DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            checkGLError_(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#endif // SERIKA_RENDERER_INCLUDE_UTILS_OPENGLUTILS_H
