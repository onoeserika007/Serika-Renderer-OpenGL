# Asset Manager 方案对齐文档

本文档用于在正式实现前对齐方案。它位于长期规划和执行规格之间：

- `architecture-analysis.md`：回答项目长期往哪里走。
- `asset-manager-proposal.md`：回答当前任务为什么这么做，取舍是什么。
- `asset-manager-design.md`：回答低阶模型/执行者具体怎么写代码。

只有本文档的方向被确认后，才应进入 `asset-manager-design.md` 级别的执行。

## 一、当前要解决的问题

当前资源加载的问题不是“缺一个高级资源系统”，而是以下几个具体痛点：

1. **重复加载**  
   同一个 mesh、shader、texture 可以被多次创建，浪费 CPU、IO 和 GPU 资源。

2. **缺少统一入口**  
   Mesh 通过 `ResourceLoader`，Shader 通过 `ShaderGLSL`，Texture 既有 CPU 缓存又有 GPU 创建路径。调用者需要知道太多细节。

3. **缺少资源统计**  
   当前无法从一个地方看到已经缓存了多少 texture / mesh / shader。

4. **后续功能缺少落脚点**  
   IBL、Bloom、profiling、场景切换、shader 缓存都会碰到资源生命周期问题。Asset Manager 是后续架构工作的前置地基，但第一版必须当下就有价值。

## 二、阶段目标

Phase 1 目标：

> 做一个同步、轻量、可用的 Asset Manager，用于统一加载并缓存 Texture / UMesh / Shader。

成功标准：

- 同一资源重复加载时命中缓存。
- `load<T>` 是真实同步加载，不是空注册。
- `AssetHandle<T>::load()` 能触发同步加载。
- `AssetHandle<T>::resolve()` 只查询，不触发加载。
- `stats()` 能统计三类已加载资源数量；只 `acquire` 但还没有 `load` 的 handle 不计入资源数量。
- 旧 `ResourceLoader` 暂时保留，现有调用路径不强制迁移。

## 三、非目标

Phase 1 不做以下内容：

- 异步加载。
- Worker thread。
- GPU command queue。
- LRU / GC / 自动卸载。
- 完整 AssetPool 类。
- Material asset 化。
- 热重载。
- 显存估算。
- Vulkan/跨后端抽象。

这些不是坏功能，但它们当前发挥不了 50% 的实际作用，会把第一版拖成空架构。

## 四、核心 API 决策

### 4.1 `load<T>` 返回资源

```cpp
std::shared_ptr<UMesh> mesh = AssetManager::getInstance().load<UMesh>(meshDesc);
```

含义：

- 同步加载。
- 如果缓存命中，直接返回缓存资源。
- 如果缓存未命中，调用旧加载逻辑创建资源并写入缓存。

这个接口服务最常见路径：调用者就是现在要用这个资源。

### 4.2 `acquire<T>` 返回 handle

```cpp
AssetHandle<Texture> handle = AssetManager::getInstance().acquire<Texture>(textureDesc);
```

含义：

- 创建或获取一个资源条目的句柄。
- 不一定立刻加载资源。
- 适合把资源引用存到材质、场景节点、配置里。

### 4.3 `handle.load()` 同步加载

```cpp
std::shared_ptr<Texture> texture = handle.load();
```

含义：

- 如果资源已加载，直接返回。
- 如果资源未加载，同步加载。
- Phase 1 没有异步，所以不存在等待后台任务。

### 4.4 `handle.resolve()` 只查询

```cpp
std::shared_ptr<Texture> texture = handle.resolve();
```

含义：

- 如果资源已加载，返回资源。
- 如果资源未加载，返回 `nullptr`。
- 不触发 IO，不创建 GPU 资源。

这两个接口必须区分清楚，否则调用方无法表达“我要现在加载”和“我只是看看有没有”。

## 五、为什么保留 Handle

虽然 Phase 1 是同步缓存，仍然保留 `AssetHandle<T>`，原因是：

- 后续 Material 可以持有 handle，而不是直接持有路径或裸 `shared_ptr`。
- handle 可以表达“资源引用”而不是“资源已经存在”。
- handle 让未来迁移异步加载时 API 形状稳定。

但 handle 第一版必须简单：

- 不做 64-bit bit packing。
- 不做自定义序列化。
- 不做异步状态机。
- 不做复杂生命周期策略。
- `release()` 只释放 handle 引用计数，不卸载资源、不删除缓存条目。

这里的“不做自定义序列化”只限制 `AssetHandle` 第一版：它暂时不参与场景/材质落盘格式设计。项目级 JSON 读写仍统一走 `Base/Json`，长期持久化数据应使用字段名 JSON object，不使用依赖字段顺序的 JSON array。

`Base/Json` 的序列化模型参考 TinyKVRaftServer RPC 的 PFR 桥接方式：

- 调用方只使用 `serializeJson<T>` / `deserializeJson<T>` 或 typed `readJsonFile/writeJsonFile`。
- 基础类型、容器、optional/map 通过 `JsonSerializer<T>` 特化递归处理。
- aggregate struct 通过 PFR 字段名反射自动遍历字段，并递归调用字段类型的 serializer。
- 普通数据结构不再逐字段写序列化代码；只有需要稳定字符串格式、兼容迁移或默认值合并的类型才保留显式逻辑。

## 六、为什么不直接用 String-Key 缓存

String-key 缓存很简单，但对 texture 不够准确：

- 同一路径可能用不同 `TextureInfo`。
- 同一路径可能用不同 `SamplerInfo`。
- 同一路径未来可能有不同 mipmap/usage 设置。

因此 Phase 1 的 Texture 缓存比较可以直接来自现有 `Texture` 的资产字段：`TextureData.path` + `TextureInfo` + `SamplerInfo`。不要再生造额外的 texture descriptor。

但这只是 AssetManager 同步缓存阶段的过渡比较方式，不是最终资产身份模型。项目确定采用 Unity-style 资产身份：`AssetGuid` 标识源资产文件，`localId` 标识源文件导入出的子对象。后续 AssetDatabase 接入后，AssetManager 的资源入口应迁移到 `AssetGuid + localId`，并通过 AssetDatabase 查询路径和导入设置。

换句话说，当前讨论过的 `AssetKey` 不作为长期 API 保留；它的内容会进入 `AssetMeta`：

- `AssetMeta`：持久化保存 GUID、路径、资源类型、导入设置和子对象表。
- `AssetObjectId`：`AssetGuid + localId`，用于引用具体可加载对象。
- AssetDatabase：用 `AssetGuid` 查 `AssetMeta`，用 path 反查 GUID。
- AssetManager：最终用 `AssetObjectId` 作为缓存索引。

Mesh 当前只有 path，但保留 `MeshDesc` 是合理的，因为后续可能加：

- import flags
- 是否多线程加载
- LOD 选项
- 坐标/法线处理策略

Shader 需要 `ShaderDesc`，至少包含：

- vertex path
- fragment path
- geometry path
- defines

## 七、缓存语义

所有 `load<T>` 必须遵守同一条原则：

> 先查缓存，再调用旧加载逻辑。

不能先加载再查缓存。否则虽然最终返回了缓存资源，但重复 IO、重复编译、重复 GPU 创建已经发生，Asset Manager 就失去意义。

正确流程：

```text
load(desc)
  -> find desc in cache
  -> if resource exists: return resource
  -> else create slot if needed
  -> call old loader
  -> store resource
  -> return resource
```

## 八、与旧系统的关系

Phase 1 不删除旧系统。

旧系统仍然存在：

- `ResourceLoader::loadTexture`
- `ResourceLoader::loadMesh`
- `ShaderGLSL::loadShader`
- `Renderer::createTexture`

Asset Manager 只是把这些旧逻辑包成统一入口，并提供缓存。

后续迁移时再逐步替换：

- `FMaterial::setTextureData(path)`
- `FScene` 中的模型/纹理加载
- `RendererOpenGL` 中部分 shader 加载路径

## 九、风险和处理

### 9.1 Descriptor 比较规则不清晰

风险：`operator==` 比较字段不清晰，会导致缓存行为难以预测。

处理：Phase 1 不写自定义 hash，先用 `operator==` 明确比较语义；后续接入 AssetDatabase 后，再把缓存索引改成 `AssetGuid + localId`。

### 9.2 Texture desc 字段不完整

风险：同一路径但不同格式/采样参数被当成同一资源。

处理：Texture key 至少包含 path、target、format、usage、mipmap、filter、wrap。

### 9.3 `resolve()` 偷偷加载

风险：调用者以为是非阻塞查询，结果触发 IO 或 GL 创建。

处理：`resolve()` 只能查缓存，未加载返回 `nullptr`。

### 9.4 `load()` 空实现或只注册

风险：API 看起来完成，但资源没有被加载。

处理：`load<T>` 必须接入真实旧加载逻辑，Texture/UMesh/Shader 三类都要覆盖。

## 十、确认后的执行文档

如果本文档方向确认，执行者应按照：

```text
docs/asset-manager-design.md
```

进行实现。

如果本文档中的 API 或非目标需要调整，应先修改本文档，再同步修改执行规格。
