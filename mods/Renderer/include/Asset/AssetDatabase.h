#ifndef SERIKA_RENDERER_INCLUDE_ASSET_ASSET_DATABASE_H
#define SERIKA_RENDERER_INCLUDE_ASSET_ASSET_DATABASE_H

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Asset/AssetMeta.h"

namespace Serika::Asset {

class AssetDatabase {
public:
	static auto getInstance() -> AssetDatabase &;

	AssetDatabase(const AssetDatabase &) = delete;
	auto operator=(const AssetDatabase &) -> AssetDatabase & = delete;

	auto clear() -> void;
	auto refresh(const std::filesystem::path &assetRoot, bool createMissingMeta) -> bool;

	auto registerMeta(const AssetMeta &meta) -> bool;
	auto createMetaForAsset(const std::filesystem::path &assetRoot, const std::filesystem::path &assetPath, AssetType type, AssetMeta &outMeta) -> bool;

	auto findMeta(const AssetGuid &guid) const -> const AssetMeta *;
	auto findGuidByPath(const std::string &sourcePath, AssetGuid &outGuid) const -> bool;
	auto containsGuid(const AssetGuid &guid) const -> bool;

	auto generateUniqueGuid() const -> AssetGuid;
	auto errors() const -> const std::vector<std::string> &;

	static auto metaPathForAsset(const std::filesystem::path &assetPath) -> std::filesystem::path;
	static auto readMetaFile(const std::filesystem::path &metaPath, AssetMeta &outMeta) -> bool;
	static auto writeMetaFile(const std::filesystem::path &metaPath, const AssetMeta &meta) -> bool;

private:
	AssetDatabase() = default;

	auto sourcePathFromAssetPath(const std::filesystem::path &assetRoot, const std::filesystem::path &assetPath, std::string &outSourcePath) -> bool;
	auto appendError(const std::string &message) -> void;

	std::map<AssetGuid, AssetMeta> metaByGuid_;
	std::map<std::string, AssetGuid> guidByPath_;
	std::vector<std::string> errors_;
};

} // namespace Serika::Asset

#endif // SERIKA_RENDERER_INCLUDE_ASSET_ASSET_DATABASE_H
