#ifndef SERIKA_RENDERER_INCLUDE_BASE_JSON_H
#define SERIKA_RENDERER_INCLUDE_BASE_JSON_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "boost/pfr/config.hpp"
#include "boost/pfr/core_name.hpp"
#include "nlohmann/json.hpp"

namespace Serika {

using JsonValue = nlohmann::json;

constexpr int JSON_INDENT = 4;

auto readJsonFile(const std::string &path, JsonValue &outJson) -> bool;
auto writeJsonFile(const std::string &path, const JsonValue &jsonValue) -> bool;

template <typename T, typename Enabled = void>
struct JsonSerializer;

template <typename T>
inline constexpr bool JsonDependentFalse = false;

template <typename T>
auto serializeJson(const T &value) -> JsonValue {
	return JsonSerializer<T>::toJson(value);
}

template <typename T>
auto deserializeJson(const JsonValue &jsonValue, T &outValue) -> bool {
	return JsonSerializer<T>::fromJson(jsonValue, outValue);
}

template <typename T>
auto readJsonFile(const std::string &path, T &outValue) -> bool {
	JsonValue jsonValue;
	if (!readJsonFile(path, jsonValue)) return false;
	return deserializeJson(jsonValue, outValue);
}

template <typename T>
auto writeJsonFile(const std::string &path, const T &value) -> bool {
	return writeJsonFile(path, serializeJson(value));
}

template <>
struct JsonSerializer<JsonValue> {
	static auto toJson(const JsonValue &value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, JsonValue &outValue) -> bool;
};

template <>
struct JsonSerializer<int> {
	static auto toJson(int value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, int &outValue) -> bool;
};

template <>
struct JsonSerializer<uint64_t> {
	static auto toJson(uint64_t value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, uint64_t &outValue) -> bool;
};

template <>
struct JsonSerializer<uint32_t> {
	static auto toJson(uint32_t value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, uint32_t &outValue) -> bool;
};

template <>
struct JsonSerializer<uint8_t> {
	static auto toJson(uint8_t value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, uint8_t &outValue) -> bool;
};

template <>
struct JsonSerializer<float> {
	static auto toJson(float value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, float &outValue) -> bool;
};

template <>
struct JsonSerializer<double> {
	static auto toJson(double value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, double &outValue) -> bool;
};

template <>
struct JsonSerializer<bool> {
	static auto toJson(bool value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, bool &outValue) -> bool;
};

template <>
struct JsonSerializer<std::string> {
	static auto toJson(const std::string &value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, std::string &outValue) -> bool;
};

template <typename T>
struct JsonSerializer<std::vector<T>> {
	static auto toJson(const std::vector<T> &values) -> JsonValue {
		JsonValue jsonValue = JsonValue::array();
		for (const T &value : values) {
			jsonValue.push_back(JsonSerializer<T>::toJson(value));
		}
		return jsonValue;
	}

	static auto fromJson(const JsonValue &jsonValue, std::vector<T> &outValues) -> bool {
		if (!jsonValue.is_array()) return false;
		std::vector<T> parsed;
		for (const JsonValue &item : jsonValue) {
			T value {};
			if (!JsonSerializer<T>::fromJson(item, value)) return false;
			parsed.push_back(value);
		}
		outValues = parsed;
		return true;
	}
};

template <typename T>
struct JsonSerializer<std::optional<T>> {
	static auto toJson(const std::optional<T> &value) -> JsonValue {
		if (!value.has_value()) return nullptr;
		return JsonSerializer<T>::toJson(*value);
	}

	static auto fromJson(const JsonValue &jsonValue, std::optional<T> &outValue) -> bool {
		if (jsonValue.is_null()) {
			outValue = std::nullopt;
			return true;
		}
		T parsed {};
		if (!JsonSerializer<T>::fromJson(jsonValue, parsed)) return false;
		outValue = parsed;
		return true;
	}
};

template <typename V>
struct JsonSerializer<std::map<std::string, V>> {
	static auto toJson(const std::map<std::string, V> &values) -> JsonValue {
		JsonValue jsonValue = JsonValue::object();
		for (auto item = values.begin(); item != values.end(); ++item) {
			jsonValue[item->first] = JsonSerializer<V>::toJson(item->second);
		}
		return jsonValue;
	}

	static auto fromJson(const JsonValue &jsonValue, std::map<std::string, V> &outValues) -> bool {
		if (!jsonValue.is_object()) return false;
		std::map<std::string, V> parsed;
		for (auto item = jsonValue.begin(); item != jsonValue.end(); ++item) {
			V value {};
			if (!JsonSerializer<V>::fromJson(item.value(), value)) return false;
			parsed[item.key()] = value;
		}
		outValues = parsed;
		return true;
	}
};

template <typename V>
struct JsonSerializer<std::unordered_map<std::string, V>> {
	static auto toJson(const std::unordered_map<std::string, V> &values) -> JsonValue {
		JsonValue jsonValue = JsonValue::object();
		for (auto item = values.begin(); item != values.end(); ++item) {
			jsonValue[item->first] = JsonSerializer<V>::toJson(item->second);
		}
		return jsonValue;
	}

	static auto fromJson(const JsonValue &jsonValue, std::unordered_map<std::string, V> &outValues) -> bool {
		if (!jsonValue.is_object()) return false;
		std::unordered_map<std::string, V> parsed;
		for (auto item = jsonValue.begin(); item != jsonValue.end(); ++item) {
			V value {};
			if (!JsonSerializer<V>::fromJson(item.value(), value)) return false;
			parsed[item.key()] = value;
		}
		outValues = parsed;
		return true;
	}
};

template <typename T>
struct JsonSerializer<T, std::enable_if_t<std::is_enum_v<T>>> {
	static auto toJson(T value) -> JsonValue {
		return static_cast<int>(value);
	}

	static auto fromJson(const JsonValue &jsonValue, T &outValue) -> bool {
		if (!jsonValue.is_number_integer()) return false;
		outValue = static_cast<T>(jsonValue.get<int>());
		return true;
	}
};

template <typename T>
struct JsonSerializer<T, std::enable_if_t<std::is_aggregate_v<T> && !std::is_array_v<T>>> {
	static auto toJson(const T &value) -> JsonValue {
#if BOOST_PFR_CORE_NAME_ENABLED
		JsonValue jsonValue = JsonValue::object();
		boost::pfr::for_each_field_with_name(value, [&jsonValue](std::string_view fieldName, const auto &field) {
			using FieldType = std::decay_t<decltype(field)>;
			jsonValue[std::string(fieldName)] = JsonSerializer<FieldType>::toJson(field);
		});
		return jsonValue;
#else
		static_assert(JsonDependentFalse<T>, "JsonSerializer aggregate support requires C++20 PFR field-name reflection.");
		return JsonValue::object();
#endif
	}

	static auto fromJson(const JsonValue &jsonValue, T &outValue) -> bool {
#if BOOST_PFR_CORE_NAME_ENABLED
		if (!jsonValue.is_object()) return false;
		T parsed {};
		bool success = true;
		boost::pfr::for_each_field_with_name(parsed, [&jsonValue, &success](std::string_view fieldName, auto &field) {
			if (!success) return;
			std::string key(fieldName);
			if (jsonValue.contains(key)) {
				using FieldType = std::decay_t<decltype(field)>;
				success = JsonSerializer<FieldType>::fromJson(jsonValue[key], field);
			}
		});
		if (!success) return false;
		outValue = parsed;
		return true;
#else
		static_assert(JsonDependentFalse<T>, "JsonSerializer aggregate support requires C++20 PFR field-name reflection.");
		return false;
#endif
	}
};

} // namespace Serika

#endif // SERIKA_RENDERER_INCLUDE_BASE_JSON_H
