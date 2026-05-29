#include "Asset/AssetDatabase.h"

#include "Base/Json.h"
#include "Utils/SRKLogger.h"

namespace Serika::Asset {

auto AssetDatabase::getInstance() -> AssetDatabase & {
	static AssetDatabase instance;
	return instance;
}

auto AssetDatabase::clear() -> void {
	metaByGuid_.clear();
	guidByPath_.clear();
	errors_.clear();
}

auto AssetDatabase::refresh(const std::filesystem::path &assetRoot, bool createMissingMeta) -> bool {
	clear();
	LOGI("AssetDatabase refresh started: root=%s createMissingMeta=%d", assetRoot.generic_string().c_str(), static_cast<int>(createMissingMeta));
	if (!std::filesystem::exists(assetRoot) || !std::filesystem::is_directory(assetRoot)) {
		appendError("Asset root does not exist: " + assetRoot.generic_string());
		return false;
	}

	std::error_code errorCode;
	std::filesystem::recursive_directory_iterator iterator(assetRoot, errorCode);
	std::filesystem::recursive_directory_iterator endIterator;
	if (errorCode) {
		appendError("Failed to scan asset root: " + assetRoot.generic_string());
		return false;
	}

	while (iterator != endIterator) {
		const std::filesystem::directory_entry &entry = *iterator;
		std::error_code fileErrorCode;
		bool isRegularFile = entry.is_regular_file(fileErrorCode);
		if (fileErrorCode) {
			appendError("Failed to inspect asset entry: " + entry.path().generic_string());
		} else if (isRegularFile && entry.path().extension() != ".meta") {
			std::filesystem::path metaPath = metaPathForAsset(entry.path());
			AssetMeta meta;
			if (std::filesystem::exists(metaPath)) {
				LOGD("AssetDatabase reading meta: asset=%s meta=%s", entry.path().generic_string().c_str(), metaPath.generic_string().c_str());
				if (!readMetaFile(metaPath, meta)) {
					appendError("Failed to read asset meta: " + metaPath.generic_string());
				} else {
					std::string sourcePath;
					if (sourcePathFromAssetPath(assetRoot, entry.path(), sourcePath)) {
						meta.sourcePath = sourcePath;
						if (!registerMeta(meta)) {
							appendError("Failed to register asset meta: " + metaPath.generic_string());
						}
					}
				}
			} else if (createMissingMeta) {
				LOGI("AssetDatabase creating missing meta: asset=%s", entry.path().generic_string().c_str());
				if (!createMetaForAsset(assetRoot, entry.path(), AssetType::Unknown, meta)) {
					appendError("Failed to create asset meta: " + entry.path().generic_string());
				}
			}
		}
		iterator.increment(errorCode);
		if (errorCode) {
			appendError("Failed to continue asset scan under: " + assetRoot.generic_string());
			return false;
		}
	}

	LOGI("AssetDatabase refresh finished: root=%s metas=%zu errors=%zu", assetRoot.generic_string().c_str(), metaByGuid_.size(), errors_.size());
	return errors_.empty();
}

auto AssetDatabase::registerMeta(const AssetMeta &meta) -> bool {
	if (!meta.guid.valid()) {
		appendError("Asset meta has invalid GUID: " + meta.sourcePath);
		return false;
	}
	if (meta.sourcePath.empty()) {
		appendError("Asset meta has empty source path: " + meta.guid.toString());
		return false;
	}

	auto guidIt = metaByGuid_.find(meta.guid);
	if (guidIt != metaByGuid_.end() && guidIt->second.sourcePath != meta.sourcePath) {
		appendError("Asset GUID conflict: " + meta.guid.toString() + " maps to both " + guidIt->second.sourcePath + " and " + meta.sourcePath);
		return false;
	}

	auto pathIt = guidByPath_.find(meta.sourcePath);
	if (pathIt != guidByPath_.end() && pathIt->second != meta.guid) {
		appendError("Asset path conflict: " + meta.sourcePath + " has multiple GUIDs");
		return false;
	}

	metaByGuid_[meta.guid] = meta;
	guidByPath_[meta.sourcePath] = meta.guid;
	LOGD("AssetDatabase registered meta: guid=%s path=%s", meta.guid.toString().c_str(), meta.sourcePath.c_str());
	return true;
}

auto AssetDatabase::createMetaForAsset(const std::filesystem::path &assetRoot, const std::filesystem::path &assetPath, AssetType type, AssetMeta &outMeta) -> bool {
	std::string sourcePath;
	if (!sourcePathFromAssetPath(assetRoot, assetPath, sourcePath)) return false;

	AssetType metaType = type;
	if (metaType == AssetType::Unknown) {
		metaType = assetTypeFromPath(sourcePath);
	}

	AssetMeta meta;
	meta.guid = generateUniqueGuid();
	meta.sourcePath = sourcePath;
	meta.type = metaType;
	meta.importSettings = defaultImportSettingsForType(metaType);

	SubAssetMeta mainAsset;
	mainAsset.localId = MAIN_ASSET_LOCAL_ID;
	mainAsset.name = assetPath.stem().string();
	mainAsset.type = metaType;
	mainAsset.importSettings = defaultImportSettingsForType(metaType);
	meta.subAssets.push_back(mainAsset);

	std::filesystem::path metaPath = metaPathForAsset(assetPath);
	if (!writeMetaFile(metaPath, meta)) {
		appendError("Failed to write asset meta: " + metaPath.generic_string());
		return false;
	}
	if (!registerMeta(meta)) return false;

	outMeta = meta;
	LOGI("AssetDatabase created meta: guid=%s source=%s meta=%s", meta.guid.toString().c_str(), meta.sourcePath.c_str(), metaPath.generic_string().c_str());
	return true;
}

auto AssetDatabase::findMeta(const AssetGuid &guid) const -> const AssetMeta * {
	auto guidIt = metaByGuid_.find(guid);
	if (guidIt == metaByGuid_.end()) return nullptr;
	return &guidIt->second;
}

auto AssetDatabase::findGuidByPath(const std::string &sourcePath, AssetGuid &outGuid) const -> bool {
	auto pathIt = guidByPath_.find(sourcePath);
	if (pathIt == guidByPath_.end()) return false;
	outGuid = pathIt->second;
	return true;
}

auto AssetDatabase::containsGuid(const AssetGuid &guid) const -> bool {
	return metaByGuid_.find(guid) != metaByGuid_.end();
}

auto AssetDatabase::generateUniqueGuid() const -> AssetGuid {
	AssetGuid guid = AssetGuid::generate();
	while (containsGuid(guid)) {
		guid = AssetGuid::generate();
	}
	return guid;
}

auto AssetDatabase::errors() const -> const std::vector<std::string> & {
	return errors_;
}

auto AssetDatabase::metaPathForAsset(const std::filesystem::path &assetPath) -> std::filesystem::path {
	return std::filesystem::path(assetPath.generic_string() + ".meta");
}

auto AssetDatabase::readMetaFile(const std::filesystem::path &metaPath, AssetMeta &outMeta) -> bool {
	bool success = Serika::readJsonFile(metaPath.string(), outMeta);
	if (!success) {
		LOGE("AssetDatabase failed to parse meta file: %s", metaPath.generic_string().c_str());
	}
	return success;
}

auto AssetDatabase::writeMetaFile(const std::filesystem::path &metaPath, const AssetMeta &meta) -> bool {
	std::filesystem::path parentPath = metaPath.parent_path();
	if (!parentPath.empty()) {
		std::error_code errorCode;
		std::filesystem::create_directories(parentPath, errorCode);
		if (errorCode) {
			LOGE("AssetDatabase failed to create meta directory: %s", parentPath.generic_string().c_str());
			return false;
		}
	}

	bool success = Serika::writeJsonFile(metaPath.string(), meta);
	if (!success) {
		LOGE("AssetDatabase failed to write meta file: %s", metaPath.generic_string().c_str());
	} else {
		LOGD("AssetDatabase wrote meta file: %s", metaPath.generic_string().c_str());
	}
	return success;
}

auto AssetDatabase::sourcePathFromAssetPath(const std::filesystem::path &assetRoot, const std::filesystem::path &assetPath, std::string &outSourcePath) -> bool {
	std::error_code errorCode;
	std::filesystem::path relativePath = std::filesystem::relative(assetPath, assetRoot, errorCode);
	if (errorCode) {
		appendError("Failed to calculate relative asset path: " + assetPath.generic_string());
		return false;
	}

	outSourcePath = relativePath.generic_string();
	return true;
}

auto AssetDatabase::appendError(const std::string &message) -> void {
	errors_.push_back(message);
	LOGE("%s", message.c_str());
}

} // namespace Serika::Asset
