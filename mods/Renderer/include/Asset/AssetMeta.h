#ifndef SERIKA_RENDERER_INCLUDE_ASSET_ASSET_META_H
#define SERIKA_RENDERER_INCLUDE_ASSET_ASSET_META_H

#include <string>
#include <vector>

#include "Asset/AssetTypes.h"
#include "Base/Json.h"

namespace Serika::Asset {

enum class AssetType {
	Unknown,
	Texture,
	Mesh,
	Shader,
	Material,
	Scene
};

struct SubAssetMeta {
	AssetLocalId localId = INVALID_ASSET_LOCAL_ID;
	std::string name;
	AssetType type = AssetType::Unknown;
	Serika::JsonValue importSettings = Serika::JsonValue::object();
};

struct AssetMeta {
	AssetGuid guid;
	std::string sourcePath;
	AssetType type = AssetType::Unknown;
	Serika::JsonValue importSettings = Serika::JsonValue::object();
	std::vector<SubAssetMeta> subAssets;
};

auto assetTypeToString(AssetType type) -> std::string;
auto assetTypeFromString(const std::string &text, AssetType &outType) -> bool;
auto assetTypeFromPath(const std::string &path) -> AssetType;

auto defaultImportSettingsForType(AssetType type) -> Serika::JsonValue;

} // namespace Serika::Asset

namespace Serika {

template <>
struct JsonSerializer<Asset::AssetGuid> {
	static auto toJson(const Asset::AssetGuid &value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, Asset::AssetGuid &outValue) -> bool;
};

template <>
struct JsonSerializer<Asset::AssetType> {
	static auto toJson(Asset::AssetType value) -> JsonValue;
	static auto fromJson(const JsonValue &jsonValue, Asset::AssetType &outValue) -> bool;
};

} // namespace Serika

#endif // SERIKA_RENDERER_INCLUDE_ASSET_ASSET_META_H
