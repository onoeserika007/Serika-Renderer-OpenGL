#include "Asset/AssetMeta.h"

#include <algorithm>
#include <cctype>
#include <filesystem>

namespace Serika::Asset {

auto assetTypeToString(AssetType type) -> std::string {
	switch (type) {
	case AssetType::Texture:
		return "Texture";
	case AssetType::Mesh:
		return "Mesh";
	case AssetType::Shader:
		return "Shader";
	case AssetType::Material:
		return "Material";
	case AssetType::Scene:
		return "Scene";
	case AssetType::Unknown:
	default:
		return "Unknown";
	}
}

auto assetTypeFromString(const std::string &text, AssetType &outType) -> bool {
	if (text == "Texture") {
		outType = AssetType::Texture;
		return true;
	}
	if (text == "Mesh") {
		outType = AssetType::Mesh;
		return true;
	}
	if (text == "Shader") {
		outType = AssetType::Shader;
		return true;
	}
	if (text == "Material") {
		outType = AssetType::Material;
		return true;
	}
	if (text == "Scene") {
		outType = AssetType::Scene;
		return true;
	}
	if (text == "Unknown") {
		outType = AssetType::Unknown;
		return true;
	}
	return false;
}

auto assetTypeFromPath(const std::string &path) -> AssetType {
	std::filesystem::path filePath(path);
	std::string extension = filePath.extension().string();
	std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) -> char {
		return static_cast<char>(std::tolower(value));
	});

	if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga" || extension == ".hdr") {
		return AssetType::Texture;
	}
	if (extension == ".obj" || extension == ".fbx" || extension == ".gltf" || extension == ".glb") {
		return AssetType::Mesh;
	}
	if (extension == ".vert" || extension == ".frag" || extension == ".geom" || extension == ".glsl") {
		return AssetType::Shader;
	}
	if (extension == ".mat") {
		return AssetType::Material;
	}
	if (extension == ".scene") {
		return AssetType::Scene;
	}
	return AssetType::Unknown;
}

auto defaultImportSettingsForType(AssetType type) -> Serika::JsonValue {
	Serika::JsonValue settings = Serika::JsonValue::object();
	settings["schemaVersion"] = 1;
	return settings;
}

} // namespace Serika::Asset

namespace Serika {

auto JsonSerializer<Asset::AssetGuid>::toJson(const Asset::AssetGuid &value) -> JsonValue {
	return value.toString();
}

auto JsonSerializer<Asset::AssetGuid>::fromJson(const JsonValue &jsonValue, Asset::AssetGuid &outValue) -> bool {
	if (!jsonValue.is_string()) return false;
	return Asset::AssetGuid::fromString(jsonValue.get<std::string>(), outValue);
}

auto JsonSerializer<Asset::AssetType>::toJson(Asset::AssetType value) -> JsonValue {
	return Asset::assetTypeToString(value);
}

auto JsonSerializer<Asset::AssetType>::fromJson(const JsonValue &jsonValue, Asset::AssetType &outValue) -> bool {
	if (!jsonValue.is_string()) return false;
	return Asset::assetTypeFromString(jsonValue.get<std::string>(), outValue);
}

} // namespace Serika
