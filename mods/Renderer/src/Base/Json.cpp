#include "Base/Json.h"

#include <exception>
#include <fstream>
#include <limits>

#include "Utils/SRKLogger.h"

namespace Serika {

auto readJsonFile(const std::string &path, JsonValue &outJson) -> bool {
	std::ifstream file(path);
	if (!file.is_open()) {
		LOGW("Failed to open JSON file for reading: %s", path.c_str());
		return false;
	}

	try {
		outJson = JsonValue::parse(file);
		return true;
	} catch (const std::exception &exception) {
		LOGE("Failed to parse JSON file: %s, error: %s", path.c_str(), exception.what());
		return false;
	} catch (...) {
		LOGE("Failed to parse JSON file: %s, unknown error", path.c_str());
		return false;
	}
}

auto writeJsonFile(const std::string &path, const JsonValue &jsonValue) -> bool {
	std::ofstream file(path);
	if (!file.is_open()) {
		LOGE("Failed to open JSON file for writing: %s", path.c_str());
		return false;
	}

	try {
		file << jsonValue.dump(JSON_INDENT) << '\n';
		return true;
	} catch (const std::exception &exception) {
		LOGE("Failed to write JSON file: %s, error: %s", path.c_str(), exception.what());
		return false;
	} catch (...) {
		LOGE("Failed to write JSON file: %s, unknown error", path.c_str());
		return false;
	}
}

auto JsonSerializer<JsonValue>::toJson(const JsonValue &value) -> JsonValue {
	return value;
}

auto JsonSerializer<JsonValue>::fromJson(const JsonValue &jsonValue, JsonValue &outValue) -> bool {
	outValue = jsonValue;
	return true;
}

auto JsonSerializer<int>::toJson(int value) -> JsonValue {
	return value;
}

auto JsonSerializer<int>::fromJson(const JsonValue &jsonValue, int &outValue) -> bool {
	if (!jsonValue.is_number_integer()) return false;
	outValue = jsonValue.get<int>();
	return true;
}

auto JsonSerializer<uint64_t>::toJson(uint64_t value) -> JsonValue {
	return value;
}

auto JsonSerializer<uint64_t>::fromJson(const JsonValue &jsonValue, uint64_t &outValue) -> bool {
	if (!jsonValue.is_number_unsigned()) return false;
	outValue = jsonValue.get<uint64_t>();
	return true;
}

auto JsonSerializer<uint32_t>::toJson(uint32_t value) -> JsonValue {
	return value;
}

auto JsonSerializer<uint32_t>::fromJson(const JsonValue &jsonValue, uint32_t &outValue) -> bool {
	if (!jsonValue.is_number_unsigned()) return false;
	uint64_t parsed = jsonValue.get<uint64_t>();
	if (parsed > std::numeric_limits<uint32_t>::max()) return false;
	outValue = static_cast<uint32_t>(parsed);
	return true;
}

auto JsonSerializer<uint8_t>::toJson(uint8_t value) -> JsonValue {
	return static_cast<uint32_t>(value);
}

auto JsonSerializer<uint8_t>::fromJson(const JsonValue &jsonValue, uint8_t &outValue) -> bool {
	if (!jsonValue.is_number_unsigned()) return false;
	uint64_t parsed = jsonValue.get<uint64_t>();
	if (parsed > std::numeric_limits<uint8_t>::max()) return false;
	outValue = static_cast<uint8_t>(parsed);
	return true;
}

auto JsonSerializer<float>::toJson(float value) -> JsonValue {
	return value;
}

auto JsonSerializer<float>::fromJson(const JsonValue &jsonValue, float &outValue) -> bool {
	if (!jsonValue.is_number()) return false;
	outValue = jsonValue.get<float>();
	return true;
}

auto JsonSerializer<double>::toJson(double value) -> JsonValue {
	return value;
}

auto JsonSerializer<double>::fromJson(const JsonValue &jsonValue, double &outValue) -> bool {
	if (!jsonValue.is_number()) return false;
	outValue = jsonValue.get<double>();
	return true;
}

auto JsonSerializer<bool>::toJson(bool value) -> JsonValue {
	return value;
}

auto JsonSerializer<bool>::fromJson(const JsonValue &jsonValue, bool &outValue) -> bool {
	if (!jsonValue.is_boolean()) return false;
	outValue = jsonValue.get<bool>();
	return true;
}

auto JsonSerializer<std::string>::toJson(const std::string &value) -> JsonValue {
	return value;
}

auto JsonSerializer<std::string>::fromJson(const JsonValue &jsonValue, std::string &outValue) -> bool {
	if (!jsonValue.is_string()) return false;
	outValue = jsonValue.get<std::string>();
	return true;
}

} // namespace Serika
