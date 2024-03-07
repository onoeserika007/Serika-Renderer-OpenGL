#ifndef UTILS_H
#define UTILS_H

#include <vector>;
#include <string>
std::vector<char> readFile(const std::string& filename);
void logDebug(const std::string& msg);
const std::string getFileExtension(const std::string& filename);
#endif // !UTILS_H
