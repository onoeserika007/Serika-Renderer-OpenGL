#ifndef SERIKA_RENDERER_INCLUDE_ASSET_ASSET_MANAGER_H
#define SERIKA_RENDERER_INCLUDE_ASSET_ASSET_MANAGER_H

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "Material/Texture.h"
#include "Utils/SRKLogger.h"

class AssetManager;
class Shader;
class UMesh;

template <typename T>
class AssetHandle {
public:
	AssetHandle();
	AssetHandle(const AssetHandle &other) = default;
	AssetHandle &operator=(const AssetHandle &other) = default;

	auto valid() const -> bool;
	explicit operator bool() const;
	auto operator==(const AssetHandle &other) const -> bool;
	auto operator!=(const AssetHandle &other) const -> bool;

	auto load() -> std::shared_ptr<T>;
	auto resolve() const -> std::shared_ptr<T>;
	auto release() -> void;

private:
	friend class AssetManager;

	AssetHandle(AssetManager *manager, std::type_index typeIndex, size_t slotIndex);

	AssetManager *manager_;
	std::type_index typeIndex_;
	size_t slotIndex_;
};

struct MeshDesc {
	std::string path;

	auto operator==(const MeshDesc &other) const -> bool {
		return path == other.path;
	}
};

struct ShaderDesc {
	std::string vertexPath;
	std::string fragmentPath;
	std::string geometryPath;
	std::vector<std::string> defines;

	auto operator==(const ShaderDesc &other) const -> bool {
		return vertexPath == other.vertexPath
		    && fragmentPath == other.fragmentPath
		    && geometryPath == other.geometryPath
		    && defines == other.defines;
	}
};

inline auto operator==(const Texture &lhs, const Texture &rhs) -> bool {
	const TextureInfo &lhsInfo = lhs.getTextureInfo();
	const TextureInfo &rhsInfo = rhs.getTextureInfo();
	const SamplerInfo &lhsSampler = lhs.getSamplerInfo();
	const SamplerInfo &rhsSampler = rhs.getSamplerInfo();
	const TextureData &lhsData = lhs.getTextureData();
	const TextureData &rhsData = rhs.getTextureData();

	return lhsData.path == rhsData.path
	    && lhsData.loadedTextureType == rhsData.loadedTextureType
	    && lhsInfo.width == rhsInfo.width
	    && lhsInfo.height == rhsInfo.height
	    && lhsInfo.type == rhsInfo.type
	    && lhsInfo.target == rhsInfo.target
	    && lhsInfo.format == rhsInfo.format
	    && lhsInfo.usage == rhsInfo.usage
	    && lhsInfo.border == rhsInfo.border
	    && lhsInfo.useMipmaps == rhsInfo.useMipmaps
	    && lhsInfo.multiSample == rhsInfo.multiSample
	    && lhsSampler.filterMin == rhsSampler.filterMin
	    && lhsSampler.filterMag == rhsSampler.filterMag
	    && lhsSampler.wrapS == rhsSampler.wrapS
	    && lhsSampler.wrapT == rhsSampler.wrapT
	    && lhsSampler.wrapR == rhsSampler.wrapR
	    && lhsSampler.borderColor == rhsSampler.borderColor;
}

struct AssetStats {
	size_t textureCount = 0;
	size_t meshCount = 0;
	size_t shaderCount = 0;
};

template <typename T>
struct AssetTraits {};

template <>
struct AssetTraits<Texture> {
	using Descriptor = Texture;
};

template <>
struct AssetTraits<UMesh> {
	using Descriptor = MeshDesc;
};

template <>
struct AssetTraits<Shader> {
	using Descriptor = ShaderDesc;
};

class AssetManager {
public:
	static auto getInstance() -> AssetManager &;

	AssetManager(const AssetManager &) = delete;
	auto operator=(const AssetManager &) -> AssetManager & = delete;

	template <typename T>
	auto load(const typename AssetTraits<T>::Descriptor &desc) -> std::shared_ptr<T>;

	template <typename T>
	auto acquire(const typename AssetTraits<T>::Descriptor &desc) -> AssetHandle<T>;

	template <typename T>
	auto resolve(const AssetHandle<T> &handle) const -> std::shared_ptr<T>;

	template <typename T>
	auto release(const AssetHandle<T> &handle) -> void;

	auto stats() const -> AssetStats;

private:
	template <typename T>
	friend class AssetHandle;

	AssetManager() = default;

	struct IStorage {
		virtual ~IStorage() = default;
		virtual auto loadedCount() const -> size_t = 0;
	};

	template <typename T>
	struct Storage : IStorage {
		struct Entry {
			typename AssetTraits<T>::Descriptor desc;
			std::shared_ptr<T> resource;
			size_t refCount = 0;
		};

		std::unordered_map<size_t, Entry> entries;
		size_t nextSlot = 1;

		auto loadedCount() const -> size_t override {
			size_t count = 0;
			for (auto entryIt = entries.begin(); entryIt != entries.end(); ++entryIt) {
				if (entryIt->second.resource != nullptr) {
					++count;
				}
			}
			return count;
		}
	};

	template <typename T>
	auto getStorage() -> Storage<T> &;

	template <typename T>
	auto findStorage() const -> const Storage<T> *;

	template <typename T>
	auto findLoadedResource(const typename AssetTraits<T>::Descriptor &desc) const -> std::shared_ptr<T>;

	template <typename T>
	auto storeLoadedResource(const typename AssetTraits<T>::Descriptor &desc, const std::shared_ptr<T> &resource) -> void;

	template <typename T>
	auto descriptorForHandle(const AssetHandle<T> &handle, typename AssetTraits<T>::Descriptor &outDesc) const -> bool;

	template <typename T>
	auto findSlotByDescriptor(const Storage<T> &storage, const typename AssetTraits<T>::Descriptor &desc) const -> size_t;

	mutable std::mutex mutex_;
	std::unordered_map<std::type_index, std::unique_ptr<IStorage>> storages_;
};

template <typename T>
AssetHandle<T>::AssetHandle()
    : manager_(nullptr), typeIndex_(std::type_index(typeid(void))), slotIndex_(0) {
}

template <typename T>
AssetHandle<T>::AssetHandle(AssetManager *manager, std::type_index typeIndex, size_t slotIndex)
    : manager_(manager), typeIndex_(typeIndex), slotIndex_(slotIndex) {
}

template <typename T>
auto AssetHandle<T>::valid() const -> bool {
	return manager_ != nullptr && slotIndex_ != 0;
}

template <typename T>
AssetHandle<T>::operator bool() const {
	return valid();
}

template <typename T>
auto AssetHandle<T>::operator==(const AssetHandle &other) const -> bool {
	return manager_ == other.manager_ && typeIndex_ == other.typeIndex_ && slotIndex_ == other.slotIndex_;
}

template <typename T>
auto AssetHandle<T>::operator!=(const AssetHandle &other) const -> bool {
	return !(*this == other);
}

template <typename T>
auto AssetHandle<T>::load() -> std::shared_ptr<T> {
	if (!valid()) {
		LOGD("AssetHandle load ignored, invalid handle: type=%s slot=%zu", typeid(T).name(), slotIndex_);
		return nullptr;
	}
	typename AssetTraits<T>::Descriptor desc;
	if (!manager_->descriptorForHandle(*this, desc)) return nullptr;
	return manager_->load<T>(desc);
}

template <typename T>
auto AssetHandle<T>::resolve() const -> std::shared_ptr<T> {
	if (!valid()) {
		LOGD("AssetHandle resolve ignored, invalid handle: type=%s slot=%zu", typeid(T).name(), slotIndex_);
		return nullptr;
	}
	return manager_->resolve(*this);
}

template <typename T>
auto AssetHandle<T>::release() -> void {
	if (!valid()) {
		LOGD("AssetHandle release ignored, invalid handle: type=%s slot=%zu", typeid(T).name(), slotIndex_);
		return;
	}
	manager_->release(*this);
	manager_ = nullptr;
	typeIndex_ = std::type_index(typeid(void));
	slotIndex_ = 0;
}

template <typename T>
auto AssetManager::acquire(const typename AssetTraits<T>::Descriptor &desc) -> AssetHandle<T> {
	std::lock_guard<std::mutex> lock(mutex_);
	Storage<T> &storage = getStorage<T>();

	size_t existingSlot = findSlotByDescriptor(storage, desc);
	if (existingSlot != 0) {
		auto entryIt = storage.entries.find(existingSlot);
		entryIt->second.refCount++;
		LOGD("AssetManager acquire existing handle: type=%s slot=%zu refCount=%zu", typeid(T).name(), existingSlot, entryIt->second.refCount);
		return AssetHandle<T>(this, std::type_index(typeid(T)), existingSlot);
	}

	size_t slotIndex = storage.nextSlot++;
	typename Storage<T>::Entry entry;
	entry.desc = desc;
	entry.resource = nullptr;
	entry.refCount = 1;
	storage.entries.emplace(slotIndex, entry);

	LOGD("AssetManager acquire new handle: type=%s slot=%zu", typeid(T).name(), slotIndex);
	return AssetHandle<T>(this, std::type_index(typeid(T)), slotIndex);
}

template <typename T>
auto AssetManager::resolve(const AssetHandle<T> &handle) const -> std::shared_ptr<T> {
	if (!handle.valid()) return nullptr;
	std::lock_guard<std::mutex> lock(mutex_);

	const Storage<T> *storage = findStorage<T>();
	if (storage == nullptr) {
		LOGD("AssetManager resolve failed, storage missing: type=%s slot=%zu", typeid(T).name(), handle.slotIndex_);
		return nullptr;
	}

	auto entryIt = storage->entries.find(handle.slotIndex_);
	if (entryIt == storage->entries.end()) {
		LOGD("AssetManager resolve failed, slot missing: type=%s slot=%zu", typeid(T).name(), handle.slotIndex_);
		return nullptr;
	}

	if (entryIt->second.resource == nullptr) {
		LOGD("AssetManager resolve returned unloaded resource: type=%s slot=%zu", typeid(T).name(), handle.slotIndex_);
	}
	return entryIt->second.resource;
}

template <typename T>
auto AssetManager::release(const AssetHandle<T> &handle) -> void {
	if (!handle.valid()) return;
	std::lock_guard<std::mutex> lock(mutex_);

	Storage<T> &storage = getStorage<T>();
	auto entryIt = storage.entries.find(handle.slotIndex_);
	if (entryIt == storage.entries.end()) {
		LOGD("AssetManager release ignored, slot missing: type=%s slot=%zu", typeid(T).name(), handle.slotIndex_);
		return;
	}

	if (entryIt->second.refCount > 0) {
		entryIt->second.refCount--;
	}
	LOGD("AssetManager release handle: type=%s slot=%zu refCount=%zu", typeid(T).name(), handle.slotIndex_, entryIt->second.refCount);
}

template <typename T>
auto AssetManager::getStorage() -> Storage<T> & {
	std::type_index typeIndex(typeid(T));
	auto storageIt = storages_.find(typeIndex);
	if (storageIt == storages_.end()) {
		std::unique_ptr<Storage<T>> storage = std::make_unique<Storage<T>>();
		Storage<T> *storagePtr = storage.get();
		storages_.emplace(typeIndex, std::move(storage));
		return *storagePtr;
	}
	return *static_cast<Storage<T> *>(storageIt->second.get());
}

template <typename T>
auto AssetManager::findStorage() const -> const Storage<T> * {
	std::type_index typeIndex(typeid(T));
	auto storageIt = storages_.find(typeIndex);
	if (storageIt == storages_.end()) return nullptr;
	return static_cast<const Storage<T> *>(storageIt->second.get());
}

template <typename T>
auto AssetManager::findLoadedResource(const typename AssetTraits<T>::Descriptor &desc) const -> std::shared_ptr<T> {
	std::lock_guard<std::mutex> lock(mutex_);
	const Storage<T> *storage = findStorage<T>();
	if (storage == nullptr) return nullptr;

	size_t slotIndex = findSlotByDescriptor(*storage, desc);
	if (slotIndex == 0) return nullptr;

	auto entryIt = storage->entries.find(slotIndex);
	if (entryIt == storage->entries.end()) return nullptr;

	return entryIt->second.resource;
}

template <typename T>
auto AssetManager::storeLoadedResource(const typename AssetTraits<T>::Descriptor &desc, const std::shared_ptr<T> &resource) -> void {
	std::lock_guard<std::mutex> lock(mutex_);
	Storage<T> &storage = getStorage<T>();

	size_t existingSlot = findSlotByDescriptor(storage, desc);
	if (existingSlot != 0) {
		auto entryIt = storage.entries.find(existingSlot);
		entryIt->second.resource = resource;
		return;
	}

	size_t slotIndex = storage.nextSlot++;
	typename Storage<T>::Entry entry;
	entry.desc = desc;
	entry.resource = resource;
	entry.refCount = 0;
	storage.entries.emplace(slotIndex, entry);
}

template <typename T>
auto AssetManager::descriptorForHandle(const AssetHandle<T> &handle, typename AssetTraits<T>::Descriptor &outDesc) const -> bool {
	if (!handle.valid()) return false;
	std::lock_guard<std::mutex> lock(mutex_);

	const Storage<T> *storage = findStorage<T>();
	if (storage == nullptr) {
		LOGD("AssetManager descriptor lookup failed, storage missing: type=%s slot=%zu", typeid(T).name(), handle.slotIndex_);
		return false;
	}

	auto entryIt = storage->entries.find(handle.slotIndex_);
	if (entryIt == storage->entries.end()) {
		LOGD("AssetManager descriptor lookup failed, slot missing: type=%s slot=%zu", typeid(T).name(), handle.slotIndex_);
		return false;
	}

	outDesc = entryIt->second.desc;
	return true;
}

template <typename T>
auto AssetManager::findSlotByDescriptor(const Storage<T> &storage, const typename AssetTraits<T>::Descriptor &desc) const -> size_t {
	for (const auto &[slotIndex, entry] : storage.entries) {
		if (entry.desc == desc) {
			return slotIndex;
		}
	}
	return 0;
}

#endif // SERIKA_RENDERER_INCLUDE_ASSET_ASSET_MANAGER_H
