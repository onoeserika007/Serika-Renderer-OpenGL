# Asset Manager Phase 1 设计与实现规格

本文档是给实现者执行的任务说明。不要扩展本文档没有要求的功能。当前阶段只解决“重复加载、缺少统一入口、缺少资源统计”这三个问题。

## 一、要解决的问题

### 1.1 Mesh 重复加载

当前 `ResourceLoader::loadMesh(path)` 每次调用都会重新走 Assimp 解析。大模型解析成本高，同一个模型重复进入场景时会浪费 CPU 和 IO。

Phase 1 要做到：

- 同一个 `MeshDesc` 多次加载，只解析一次。
- 后续加载直接返回缓存里的 `std::shared_ptr<UMesh>`。

### 1.2 Shader 重复加载/编译

当前 `ShaderGLSL::loadShader(vs, fs)` 会创建新的 shader 对象，调用方再 `compileAndLink()`。很多默认 shader 工厂方法本质上会重复加载和编译。

Phase 1 要做到：

- 同一个 `ShaderDesc` 多次加载，只创建和编译一次。
- 后续加载直接返回缓存里的 `std::shared_ptr<Shader>`。

### 1.3 Texture GPU 对象重复创建

`ResourceLoader::loadTexture(path)` 已经有 CPU 图片缓存，但 GPU `Texture` 对象仍可能重复创建。

Phase 1 要做到：

- 同一个 `Texture` 资产描述多次加载，只创建一次 GPU texture。
- 后续加载直接返回缓存里的 `std::shared_ptr<Texture>`。

### 1.4 缺少统一资源统计

现在无法从一个地方看到当前 AssetManager 管理了多少资源。

Phase 1 要做到：

- `AssetManager::stats()` 能返回 Texture / Mesh / Shader 的已加载资源数量。
- `stats()` 统计已实际加载的资源数量；只 `acquire` 但还没有 `load` 的空条目不计入资源数量。
- 暂不统计显存大小。

## 二、明确不做什么

以下内容不要在 Phase 1 实现，也不要写额外接口占位：

- 异步加载。
- Worker thread。
- GPU command queue。
- LRU / GC / 自动淘汰。
- AssetPool 独立类。
- Material 资产化。
- 热重载。
- Vulkan/跨后端抽象。
- 64-bit bit packing handle。
- 显存估算。

如果实现过程中觉得需要这些能力，先停止并重新讨论，不要自行加入。

## 三、最终 API

### 3.1 Include

新增一个头文件：

```cpp
#include "Asset/AssetManager.h"
```

### 3.2 一步式同步加载

```cpp
AssetManager& assetManager = AssetManager::getInstance();

TextureInfo textureInfo;
textureInfo.target = TextureTarget_TEXTURE_2D;
textureInfo.format = TextureFormat_RGBA8;
textureInfo.usage = TextureUsage_Sampler;

SamplerInfo samplerInfo;
samplerInfo.filterMin = Filter_LINEAR;
samplerInfo.filterMag = Filter_LINEAR;
samplerInfo.wrapS = Wrap_REPEAT;
samplerInfo.wrapT = Wrap_REPEAT;

TextureData textureData;
textureData.path = "assets/texture/pbr/rusted_iron/albedo.png";

Texture textureDesc(textureInfo, samplerInfo, textureData);

std::shared_ptr<Texture> texture = assetManager.load<Texture>(textureDesc);
```

```cpp
MeshDesc meshDesc;
meshDesc.path = "assets/objects/bunny.obj";

std::shared_ptr<UMesh> mesh = assetManager.load<UMesh>(meshDesc);
```

```cpp
ShaderDesc shaderDesc;
shaderDesc.vertexPath = "assets/shader/Passes/PlainPass/BaseColor.vert";
shaderDesc.fragmentPath = "assets/shader/Passes/PlainPass/BaseColor.frag";

std::shared_ptr<Shader> shader = assetManager.load<Shader>(shaderDesc);
```

### 3.3 Handle 用法

Handle 是缓存条目的引用，不负责异步。

```cpp
AssetHandle<Texture> handle = assetManager.acquire<Texture>(textureDesc);

std::shared_ptr<Texture> texture = handle.load();
std::shared_ptr<Texture> cached = handle.resolve();

handle.release();
```

语义：

- `acquire<T>(desc)`：拿到资源句柄；如果缓存里没有资源，也要记录 desc。
- `handle.load()`：同步加载；如果已经加载过，直接返回缓存资源。
- `handle.resolve()`：只查询，不触发加载；如果资源还没加载，返回 `nullptr`。
- `handle.release()`：释放该 handle 的引用计数。Phase 1 不做卸载、GC、LRU，因此 release 不删除缓存条目，也不销毁已加载资源。

### 3.4 统计

```cpp
AssetStats stats = assetManager.stats();
size_t textureCount = stats.textureCount;
size_t meshCount = stats.meshCount;
size_t shaderCount = stats.shaderCount;
```

Phase 1 只要求已加载资源数量正确，不要求统计 acquire 后尚未 load 的空条目。

## 四、数据结构

### 4.1 Descriptor

在 `AssetManager.h` 中定义以下 descriptor。

Texture 本身就是 texture 的资产描述载体，包含 `TextureInfo`、`SamplerInfo`、`TextureData`，因此不要再定义额外的 texture descriptor。

注意：descriptor 只是 Phase 1 同步缓存阶段的过渡输入，不是长期资产身份。长期路线中，这些描述字段会进入 `AssetMeta` 的 import settings；AssetManager 缓存索引会迁移到 `AssetObjectId = AssetGuid + localId`。

比较字段必须至少包含：

- `path`
- `info.target`
- `info.format`
- `info.usage`
- `info.useMipmaps`
- `sampler.filterMin`
- `sampler.filterMag`
- `sampler.wrapS`
- `sampler.wrapT`
- `sampler.wrapR`

```cpp
struct MeshDesc {
    std::string path;

    bool operator==(const MeshDesc& other) const;
};
```

`MeshDesc` 现在只有 path，但保留 struct，方便后续增加 import flags、是否使用线程池、LOD 参数等。

```cpp
struct ShaderDesc {
    std::string vertexPath;
    std::string fragmentPath;
    std::string geometryPath;
    std::vector<std::string> defines;

    bool operator==(const ShaderDesc& other) const;
};
```

`ShaderDesc` 必须包含 geometry path，避免 geometry shader 和非 geometry shader 使用同一个缓存 key。

### 4.2 Descriptor 比较

Phase 1 不为 `Texture`、`MeshDesc`、`ShaderDesc` 提供 `std::hash` 特化。

要求：

- 先用 `operator==` 明确 descriptor 的相等语义。
- 缓存查找可以遍历当前类型的 `entries`。
- 后续接入 AssetDatabase 后，再把查找入口改成 `AssetGuid + localId`。
- `AssetGuid` 只标识源资产文件；缓存具体对象时必须使用 `AssetGuid + localId`，避免无法区分同一模型文件导入出的多个 mesh/material/animation。

### 4.3 AssetTraits

用 traits 把资源类型映射到 descriptor。

```cpp
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
```

## 五、内部存储

不要实现独立 `AssetPool` 文件。Phase 1 在 `AssetManager` 内部使用简单 map。

每种资源类型一个 `Storage<T>`：

```cpp
template <typename T>
struct Storage {
    struct Entry {
        typename AssetTraits<T>::Descriptor desc;
        std::shared_ptr<T> resource;
        size_t refCount = 0;
    };

    std::unordered_map<size_t, Entry> entries;
    size_t nextSlot = 1;
};
```

说明：

- slot 0 保留为 invalid handle。
- 根据 desc 查缓存时遍历 `entries` 并使用 `operator==` 比较。
- `entries` 用于根据 handle 查资源。
- `resource == nullptr` 表示已 acquire 但尚未 load。

## 六、类职责

### 6.1 AssetManager

`AssetManager` 是单例。

必须提供：

```cpp
class AssetManager {
public:
    static AssetManager& getInstance();

    template <typename T>
    std::shared_ptr<T> load(const typename AssetTraits<T>::Descriptor& desc);

    template <typename T>
    AssetHandle<T> acquire(const typename AssetTraits<T>::Descriptor& desc);

    template <typename T>
    std::shared_ptr<T> resolve(const AssetHandle<T>& handle) const;

    template <typename T>
    void release(const AssetHandle<T>& handle);

    AssetStats stats() const;
};
```

`load<T>` 行为：

1. 根据 desc 查缓存。
2. 如果已存在且 `resource != nullptr`，直接返回。
3. 如果不存在，创建 slot。
4. 调用对应的旧加载逻辑创建资源。
5. 写回缓存。
6. 返回资源。

注意：必须先查缓存，再调用旧加载逻辑。不能先加载再查缓存，否则缓存不会减少重复 IO/编译。

### 6.2 AssetHandle

`AssetHandle<T>` 可以放在 `AssetManager.h` 里，不单独拆文件。

必须提供：

```cpp
template <typename T>
class AssetHandle {
public:
    AssetHandle();

    bool valid() const;
    explicit operator bool() const;

    std::shared_ptr<T> load();
    std::shared_ptr<T> resolve() const;
    void release();
};
```

不要提供 `loadAsync`。Phase 1 不做异步，保留空函数会误导调用方。

`AssetHandle<T>::load()` 调用 `AssetManager::load<T>(desc)` 或等价内部函数，必须能触发同步加载。

`AssetHandle<T>::resolve()` 只能查询，不能触发加载。

## 七、三类资源的加载逻辑

### 7.1 Texture

`AssetManager::load<Texture>(Texture)` 使用现有逻辑：

1. `ResourceLoader::getInstance().loadTexture(desc.path)` 读取 CPU 图片。
2. 构造 `TextureData`：
   - `unitDataArray.push_back(cpuData)`
   - `path = desc.path`
   - `loadedTextureType` 如果能从使用场景得知则设置；否则 Phase 1 可保持默认。
3. 通过 `Application::getRenderer()` 获取 renderer。
4. 调用 `renderer->createTexture(desc.info, desc.sampler, textureData)`。
5. 缓存并返回 `std::shared_ptr<Texture>`。

必须保证同一 desc 第二次 `load<Texture>` 不再调用 `createTexture`。

### 7.2 Mesh

`AssetManager::load<UMesh>(MeshDesc)` 使用现有逻辑：

```cpp
ResourceLoader::getInstance().loadMesh(desc.path)
```

必须保证同一 desc 第二次 `load<UMesh>` 不再调用 `loadMesh`。

### 7.3 Shader

`AssetManager::load<Shader>(ShaderDesc)` 使用现有逻辑：

```cpp
std::shared_ptr<ShaderGLSL> shader =
    ShaderGLSL::loadShader(desc.vertexPath, desc.fragmentPath, desc.geometryPath);
shader->compileAndLink();
```

必须保证同一 desc 第二次 `load<Shader>` 不再调用 `compileAndLink()`。

`defines` 暂时只参与 cache key，不要求真正注入 shader 编译。后续 shader variant 再处理。

## 八、文件改动

### 8.1 新增

```text
mods/Renderer/include/Asset/AssetManager.h
mods/Renderer/src/Asset/AssetManager.cpp
```

不要新增 `AssetHandle.h`。

### 8.2 修改

```text
mods/Renderer/CMakeLists.txt
```

如果 CMake 已经 `GLOB_RECURSE` 收集 `src/*.cpp`，则只需要确保 include 路径已覆盖 `mods/Renderer/include`，不要额外手写源文件列表。

### 8.3 暂不修改

```text
mods/Renderer/include/Base/ResourceLoader.h
mods/Renderer/src/Base/ResourceLoader.cpp
mods/Renderer/src/FScene.cpp
mods/Renderer/src/Materials/FMaterial.cpp
```

Phase 1 先让新 API 可用，并用最小验证代码确认缓存语义。旧调用点迁移放到后续任务。

## 九、实现步骤

按以下顺序实现，不要跳步：

1. 新建 `AssetManager.h`。
2. 定义 descriptor、AssetStats、AssetTraits。
3. 在同一个头文件中定义 `AssetHandle<T>`。
4. 在 `AssetManager` 中定义内部 `Storage<T>`。
5. 实现 `acquire<T>`、`resolve<T>`、`release<T>`。
6. 实现 `AssetHandle<T>::load/resolve/release`。
7. 新建 `AssetManager.cpp`。
8. 实现 `AssetManager::getInstance()` 和 `stats()`。
9. 在 `.cpp` 中显式特化：
   - `load<Texture>`
   - `load<UMesh>`
   - `load<Shader>`
10. 编译。
11. 修复所有 C4267/C4244/C4305/C4018 警告。
12. 做最小验证。

## 十、最小验证

### 10.1 编译验证

必须通过：

```powershell
cmake --build build --parallel 30
```

并确认没有以下警告：

```text
C4267
C4244
C4305
C4018
```

### 10.2 行为验证

建议新增临时验证代码或最小单元测试，验证以下行为：

```cpp
AssetManager& assetManager = AssetManager::getInstance();

MeshDesc meshDesc;
meshDesc.path = "assets/objects/bunny.obj";

std::shared_ptr<UMesh> meshA = assetManager.load<UMesh>(meshDesc);
std::shared_ptr<UMesh> meshB = assetManager.load<UMesh>(meshDesc);

assert(meshA == meshB);
```

Texture 和 Shader 同理：

- 同一 `Texture` 资产描述两次 load 返回同一个 `shared_ptr<Texture>`。
- 同一 `ShaderDesc` 两次 load 返回同一个 `shared_ptr<Shader>`。
- `acquire<T>(desc).resolve()` 在未 load 前返回 `nullptr`。
- `acquire<T>(desc).load()` 会同步加载并返回资源。
- load 后 `resolve()` 返回同一资源。
- `stats()` 数量正确。

## 十一、完成标准

该任务完成必须同时满足：

- AssetManager API 编译可用。
- `Texture` / `UMesh` / `Shader` 三类资源都有真实 `load<T>` 实现。
- 同一 desc 重复 load 命中缓存，不重复创建资源。
- `AssetHandle<T>::load()` 能触发同步加载。
- `AssetHandle<T>::resolve()` 不触发加载。
- 没有 `loadAsync` 空实现。
- 没有新增 worker、GC、LRU、Material asset 等非 Phase 1 内容。
- MSVC 截断相关警告为 0。

## 十二、后续任务，不在本次实现

以下内容只记录方向，不要在 Phase 1 做：

- 把 `FMaterial::setTextureData(path)` 迁移到 AssetManager。
- 把场景工厂里的 mesh/shader/texture 加载迁移到 AssetManager。
- 增加资源显存估算。
- 增加资源卸载策略。
- 增加异步加载。
- 增加 shader variant/defines 实际编译支持。
