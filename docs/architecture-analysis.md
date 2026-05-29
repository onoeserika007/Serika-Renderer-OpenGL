# Serika Renderer 长期架构规划与迭代路线图

本文档用于确定 Serika Renderer 的长期改进方向和阶段性待办事项。它不是单纯的功能愿望清单，而是一个分层规划：

- 长期目标：Serika 最终要成为怎样的渲染器。
- 主要矛盾：当前最限制项目继续演进的问题是什么。
- 改革方向：应该优先重构哪些架构能力。
- 阶段计划：接下来几个阶段分别交付什么。
- 专项任务：功能、性能、工具链、benchmark 如何推进。

## 一、当前架构总览

| 维度 | 现状 |
|---|---|
| 图形 API | OpenGL 4.3 Core Profile |
| 渲染管线 | Forward + Deferred，可通过配置切换 |
| 光追 | CPU Path Tracer，OpenMP 并行，当前偏离线验证用途 |
| 材质 | 已有 PBR/Disney BRDF 参数，材质与 shader/resource 绑定仍较直接 |
| 阴影 | Directional shadow + PCF/PCSS，Point light cube shadow |
| 后处理 | HDR tone mapping、SSAO 基础实现 |
| 场景加载 | Assimp 加载 OBJ/GLTF，已有对象/Transform 层级 |
| 加速结构 | BVH 已有，中点分割为主，SAH 相关枚举存在但未真正落地 |
| UI/调试 | Dear ImGui 调试面板，若干 debug primitive/BVH 可视化 |
| Profiling | 主要依赖 CPU 计时宏，缺少稳定的 GPU pass profiling |
| 资源管理 | ResourceLoader、ShaderGLSL、RendererOpenGL、Material 各自管理资源，缺少统一生命周期与统计 |

## 二、长期目标

### 2.1 总目标

Serika Renderer 的长期目标不是一次性追平大型商业引擎，而是成为一个结构清晰、可实验、可验证的现代实时渲染器：

- 具备完整的实时渲染主线：PBR、IBL、Shadow、Deferred/Forward、后处理、透明、debug visualization。
- 具备可扩展的架构主线：统一资源管理、清晰 RenderPass/RenderGraph、可插拔 shader/material 系统。
- 具备可靠的性能评估能力：CPU/GPU 分项 profiling、场景 benchmark、可重复的画质/性能对比。
- 保留研究型扩展空间：CPU/GPU path tracing、BVH 优化、采样算法、GI 实验。

### 2.2 长期形态

理想状态下，项目应形成以下层次：

```text
Application / Viewer / UI
        |
Scene / Camera / Light / Object
        |
Material / Asset / Shader
        |
RenderGraph / RenderPass
        |
Renderer Backend (OpenGL first, Vulkan optional)
        |
GPU Resources / Profiling / Debug Tools
```

OpenGL 仍然是当前主后端，不应过早为 Vulkan 做大规模抽象。但设计新模块时，要避免把 OpenGL 细节泄漏到上层业务对象里。

## 三、当前主要矛盾

当前项目的主要矛盾是：

> 渲染功能已经开始变多，但资源、Pass、Shader、Profiling 等基础架构还没有形成稳定边界，导致继续添加功能时成本越来越高，性能问题也难以定位。

具体表现如下。

### 3.1 资源生命周期不统一

纹理、Mesh、Shader、FBO、UBO、VAO/VBO 的创建和销毁分散在不同类中。当前无法稳定回答：

- 当前加载了多少纹理、mesh、shader。
- 哪些资源来自同一个文件，是否重复加载。
- 某个场景退出后哪些资源应该释放，哪些应该常驻。
- GPU 资源大概占用多少显存。

这直接影响场景切换、资源复用、加载卡顿、内存控制。

### 3.1.1 持久化格式需要统一边界

ProjectConfig、AssetMeta、后续场景/材质文件都会写 JSON。如果每个业务模块直接使用具体 JSON 库，就会反复实现格式化、错误处理和 schema 兼容逻辑。

项目级约定：

- 业务代码统一通过 `Base/Json` 读写 JSON，不直接依赖具体 JSON 库的 parse/dump 细节。
- 可以使用 Boost.PFR 做静态反射序列化，但长期落盘格式必须写成字段名 JSON object。
- `Base/Json` 的核心桥接方式参考 TinyKVRaftServer RPC：顶层入口只调用 `JsonSerializer<T>`；基础类型、容器、optional/map 用模板特化处理；普通 aggregate struct 由 PFR 遍历字段并递归调用字段类型的 `JsonSerializer<FieldType>`。
- 对字段类型全部已被支持的 aggregate struct，不再编写逐字段序列化代码。只有需要特殊语义的类型，例如 GUID 字符串格式、枚举可读名称、版本迁移/default merge，才写显式特化或显式读取逻辑。
- 如果 class 同时包含可持久化字段和运行期状态，应把可持久化字段拆成独立 aggregate 组件，由原 class 持有或继承该组件；不要在 serializer 里堆“忽略字段”规则。
- 不使用“按字段顺序写 JSON array”作为持久化格式；这种方式适合 RPC，但不适合会跨版本演进的配置和资产元数据。

### 3.2 RenderPass 和资源依赖不显式

Forward、Deferred、SSAO、Shadow、ToScreen 等 pass 已经存在，但输入输出关系主要靠代码约定。随着 Bloom、IBL、TAA、透明、debug overlay 等功能加入，pass 之间的依赖会继续复杂化。

短期不应直接上完整 RenderGraph，但必须先让 pass 的资源边界更清楚。

### 3.3 Shader/Material 系统耦合较重

ShaderGLSL 直接承担 shader 加载、编译、program 管理；Material 又持有 shader resources、texture data、uniform block 等状态。这个结构在功能少时够用，但后续 shader variant、PBR/IBL、deferred material、post-process shader 都会挤在一起。

### 3.4 缺少 GPU profiling 和 benchmark

没有稳定的 GPU pass 时间、draw call、资源统计，就无法判断优化是否真的有效。Instancing、culling、UBO 脏标记、shader 缓存这些优化都需要测量闭环。

### 3.5 功能路线和架构路线混在一起

Bloom、FXAA、IBL 等功能确实重要，但如果在基础设施不稳时直接堆功能，会加剧耦合。项目应该分清：

- 架构改革：让后续功能更容易做。
- 功能建设：补齐现代渲染器能力。
- 性能治理：让功能在真实场景中可用。
- 验证体系：证明改动有价值。

## 四、改革方向

### 4.1 资源治理优先

第一个架构改革应是 Asset Manager，但它必须小步落地，不做空转设计。

资源治理路线必须先确定资产身份模型，避免 Phase 1 的临时缓存 key 变成后续重构债务。项目确定采用 Unity-style AssetDatabase/Library 机制：

- 源资产仍保存在 `assets/`。
- 资产身份由持久化 `AssetGuid` + `localId` 组成。
- `AssetGuid` 标识源资产文件，`localId` 标识源文件导入出的子对象。
- 元数据保存资产身份、导入设置和派生对象信息。
- `Library` 保存可删除、可重建的导入缓存和索引。
- AssetDatabase 启动/刷新时扫描源资产与元数据，建立 `guid -> path`、`path -> guid`、派生对象索引，并检测 GUID 冲突。
- AssetManager 负责运行时加载与缓存，不负责生成资产身份。
- Asset GUID 必须独立实现，不能复用现有 `Serika::UUID<T>` 运行期递增计数器。

`AssetKey` 不作为长期概念保留。当前设计里用于描述加载参数的内容，应收敛进 `AssetMeta` 和 import settings：

- `AssetMeta` 是持久化元数据，保存 `AssetGuid`、源路径、资源类型、导入设置和子对象表。
- `AssetObjectId` 是运行时引用和缓存索引，结构为 `AssetGuid + localId`。
- AssetDatabase 的主索引是 `AssetGuid -> AssetMeta`，辅助索引是 `path -> AssetGuid`。
- AssetManager 的长期缓存索引是 `AssetObjectId -> loaded resource`。
- 当前阶段仍按 raw data 导入。`AssetMeta.importSettings` 只保存 schema version，不提前加入未接入的 importer 分化或导入后处理选项。Texture compression/mipmap、Mesh import flags、Shader variant 等字段等真正接入导入处理时再加入。

第一阶段只解决当前能发挥价值的问题：

- 统一 Texture / UMesh / Shader 的同步加载入口。
- 缓存已加载资源，避免重复 IO、重复 Assimp 解析、重复 shader 编译。
- 提供资源数量统计。
- 保持旧 ResourceLoader 可用，逐步迁移调用点。
- API 形状必须能平滑接入后续 AssetDatabase，不引入必然被删除的临时身份模型。

暂不做：

- Worker thread 异步加载。
- LRU/GC。
- Material asset 化。
- 跨线程 GL command queue。
- 完整 AssetPool 抽象。
- 完整 AssetDatabase/Library 实现。

这些可以作为后续阶段，但不进入第一版核心设计。

### 4.2 先 profiling，再性能优化

性能优化要先有测量。下一批架构任务应加入：

- CPU frame scope 统计。
- GPU `GL_TIME_ELAPSED` pass query。
- draw call / triangle / texture / shader 数量统计。
- Debug UI 展示每个 pass 的时间。

有 profiling 之后再推进 culling、instancing、UBO 脏标记，避免凭感觉优化。

### 4.3 Pass 边界显式化，暂缓完整 RenderGraph

完整 RenderGraph 是长期目标，但当前直接上会过重。更合适的路径是：

1. 让每个 RenderPass 明确声明输入/输出资源。
2. 统一 pass 创建和 resize 行为。
3. 统一 framebuffer/texture attachment 的命名与调试输出。
4. 当 pass 数量继续增长后，再抽象 RenderGraph。

### 4.4 功能补齐围绕 PBR 主线展开

功能路线不要散。优先围绕一个现代实时 PBR renderer 的主线补齐：

1. IBL：irradiance map、prefiltered env map、BRDF LUT。
2. Bloom：HDR 后处理链路补齐。
3. FXAA/TAA：抗锯齿与 temporal 基础。
4. CSM：主光源阴影质量。
5. 透明/OIT：补齐常见渲染对象类型。

粒子、Decal、GI 等放到更后面，避免路线发散。

### 4.5 Path Tracing 作为研究线，不阻塞实时线

CPU path tracer、BVH、MIS、GPU path tracer 很有价值，但它们和实时渲染主线的架构诉求不同。建议作为并行研究线推进：

- 短期先做 SAH BVH + MIS，提升已有 CPU path tracer 的正确性和质量。
- GPU path tracer 等待资源管理、shader/compute 基础更稳后再做。

## 五、阶段计划

### Phase 0：整理与决策校准

目标：先把方向定稳，避免边做边推翻。

交付：

- 更新本文档，明确长期目标和阶段计划。
- 收敛 `asset-manager-design.md`，只保留下一阶段实际要做的 Asset Manager 设计。
- 清理已否决的设计项，例如过早的 worker、LRU、Material asset、GPU command queue。

验收：

- 文档中的 Phase 1 和当前要做的任务一致。
- API 示例和项目真实类型一致。
- 每个设计项都能说明当前阶段至少 50% 的实际价值。

### Phase 1：Asset Manager 同步缓存与统计

目标：解决资源重复加载和资源不可见问题，为后续功能扩张打底。

范围：

- `Texture`：第一版实现可临时通过路径 + `TextureInfo` + `SamplerInfo` 查找缓存，但 API 设计必须预留迁移到 `AssetGuid + localId`。
- `UMesh`：基于 `MeshDesc` 缓存。
- `Shader`：基于 `ShaderDesc` 缓存。
- `AssetManager::load<T>(desc)` 返回 `shared_ptr<T>`。
- `AssetManager::acquire<T>(desc)` 返回 `AssetHandle<T>`。
- `AssetHandle<T>::load()` 同步加载并返回资源。
- `AssetHandle<T>::resolve()` 非阻塞查询。
- `AssetStats` 先统计资源数量，显存估算后置。

暂不做：

- 异步加载。
- LRU/GC。
- Material asset。
- GPU command queue。
- 完整 AssetPool。

验收：

- 同一资产描述多次 `load` 返回同一缓存资源；后续 AssetDatabase 接入后应切换为同一 `AssetGuid + localId` 命中同一资源。
- Mesh 不重复 Assimp 解析。
- Shader 不重复编译。
- Texture 不重复创建 GPU texture。
- stats 能正确统计 Texture / UMesh / Shader 数量。
- 不破坏现有 ResourceLoader 调用路径。

### Phase 2：Profiling 与可观测性

目标：让后续性能优化有数据闭环。

任务：

- 增加 CPU frame/pass scope profiler。
- 增加 OpenGL GPU timer query。
- 统计 draw call、triangle count、active texture/shader 数量。
- ImGui 面板展示每帧 CPU/GPU pass 时间。
- 输出可保存的 benchmark CSV/JSON。

验收：

- 每个主要 RenderPass 有 GPU 时间。
- 同一场景连续运行的统计稳定。
- 可以用数据判断某次优化是否有效。

### Phase 2.5：AssetDatabase 与 Library 索引

目标：建立项目级资产身份与可重建资产库，让 AssetManager 的资源入口从临时 descriptor 逐步迁移到稳定资产身份。

范围：

- 定义资产身份模型：`AssetGuid` 标识源资产文件，`localId` 标识源文件导入出的子对象。
- 独立实现 Asset GUID 生成方法，不复用 `Serika::UUID<T>`。
- 定义资产元数据文件格式，保存 GUID、资源类型、导入设置和子对象表。
- 定义 `AssetObjectId = AssetGuid + localId`，作为资源引用和 AssetManager 长期缓存索引。
- 定义第一版 import settings schema：当前仅保留 `schemaVersion`。不要提前加入尚未接入的 importer 分化、texture compression、mipmap、mesh import flags、shader variant 等后处理字段。
- 建立 `Library` 目录，用于保存可删除、可重建的资产索引和导入缓存。
- 实现 AssetDatabase 启动/刷新流程：扫描 `assets/` 和元数据，建立 `guid -> path`、`path -> guid` 索引。
- 检测 GUID 冲突：同一 GUID 指向不同路径时报告冲突，不静默选择。
- 缺失元数据时生成新 GUID；生成后必须查当前索引，碰撞则重新生成。
- 为 Mesh/Texture 等可能产生多个子对象的资源预留 `localId`。

暂不做：

- 编辑器 UI。
- 自动修复所有引用。
- 异步导入管线。
- 跨平台压缩缓存。

验收：

- 删除 `Library` 后可以通过扫描 `assets/` 和元数据重建索引。
- 同一个 GUID 出现在不同路径时能稳定报错。
- 移动资产时，只要元数据跟随移动，GUID 不变。
- AssetManager 可以通过 AssetDatabase 查询到资产路径，并继续走现有加载逻辑。

### Phase 3：低风险性能治理

目标：做确定性强、收益清楚的性能优化。

任务：

- 视锥体剔除：使用已有 Frustum/BBox 体系过滤不可见对象。
- UBO 脏标记：避免每个 draw 重复上传未变化数据。
- Mesh/material 分组：为 instancing 和 batching 做准备。
- 基础 instancing：先覆盖相同 mesh + material 的简单场景。

验收：

- Sponza/Bistro 类场景 draw call 或 GPU time 有可测下降。
- Debug UI 能显示剔除前后对象数量。
- 优化不改变画面结果。

### Phase 4：PBR 功能主线补齐

目标：把 Serika 的实时渲染能力推进到现代 PBR renderer 的基础水位。

任务：

- IBL diffuse irradiance map。
- Prefiltered environment map。
- BRDF LUT。
- Bloom。
- FXAA，后续评估 TAA。
- CSM。

验收：

- PBR test scene 能用 IBL 正确呈现金属/粗糙度差异。
- HDR + Bloom + tone mapping 链路稳定。
- 阴影覆盖更大场景时质量明显改善。

### Phase 5：RenderPass 资源边界与 RenderGraph 预备

目标：让后续后处理、透明、GI 等功能不会继续堆硬编码。

任务：

- 每个 pass 显式声明输入/输出。
- 统一 framebuffer resize / recreate。
- 统一 pass debug name。
- 建立 pass resource registry。
- 评估是否进入完整 RenderGraph。

验收：

- 新增一个后处理 pass 不需要手动穿透多个 renderer 成员变量。
- resize、调试、资源释放路径一致。

### Phase 6：研究线与高级功能

目标：在主架构稳定后推进研究型能力。

任务：

- CPU path tracer：SAH BVH、MIS、材质采样改进。
- GPU path tracer：compute shader 原型。
- OIT / Decal / Particle。
- Bindless textures。
- Multi-draw indirect。
- Vulkan 后端可行性评估。

验收：

- 研究功能不破坏实时渲染主线。
- 每个高级功能都有 benchmark 或画质对比。

## 六、专项任务池

### 6.1 架构专项

| 任务 | 优先级 | 说明 |
|---|---|---|
| Asset Manager 同步版 | P0 | 当前下一步任务 |
| AssetDatabase / Library 索引 | P0 | 资源身份路线，AssetManager 后续演进前置条件 |
| Profiling 基础设施 | P0 | 后续优化前置条件 |
| RenderPass 边界显式化 | P1 | RenderGraph 前置条件 |
| Shader variant 管理 | P1 | PBR/IBL/TAA 后会需要 |
| RenderGraph | P2 | Pass 数量继续增长后再做 |
| Vulkan 抽象 | P3 | 暂不作为近期目标 |

### 6.2 功能专项

| 任务 | 优先级 | 说明 |
|---|---|---|
| IBL | P0 | PBR 主线核心 |
| Bloom | P1 | HDR 后处理链路 |
| FXAA | P1 | 低成本抗锯齿 |
| CSM | P1 | 大场景阴影 |
| TAA | P2 | 需要 motion vector / history |
| OIT | P2 | 透明对象支持 |
| Decal / Particle | P3 | 表现力扩展 |

### 6.3 性能专项

| 任务 | 优先级 | 说明 |
|---|---|---|
| 视锥体剔除 | P0 | 低风险，容易验证 |
| UBO 脏标记 | P0 | 减少重复上传 |
| Instancing | P1 | 需要 mesh/material 分组 |
| Shader 缓存/预编译 | P1 | 降低启动和切换卡顿 |
| Bindless texture | P2 | OpenGL 高级优化 |
| Multi-draw indirect | P3 | 需要更成熟的 draw submission |

### 6.4 Path Tracing 专项

| 任务 | 优先级 | 说明 |
|---|---|---|
| SAH BVH | P1 | 提升遍历效率 |
| MIS | P1 | 提升收敛质量 |
| SIMD traversal | P2 | CPU 性能优化 |
| GPU path tracer | P3 | 等 shader/compute 体系更稳 |

## 七、Benchmark 与验收体系

Benchmark 要分组，不把不同性质的渲染器混在一个结论里。

### 7.1 实时栅格化 benchmark

目标：比较实时渲染性能和 pass 成本。

场景：

- Sponza：基础 PBR + 阴影 + IBL。
- Bistro：高多边形、多材质、draw call 压力。
- San Miguel：大场景与光照压力。

指标：

- CPU frame time。
- GPU frame time。
- 每个 RenderPass GPU time。
- draw call 数。
- triangle 数。
- visible / culled object 数。
- texture / shader / mesh 数量。

参考对象：

- Godot 4 或 Falcor 的同类实时场景作为参考，不追求完全同画面，只比较同类负载下的量级。

### 7.2 Path tracing benchmark

目标：比较采样质量、收敛速度和加速结构效率。

场景：

- Cornell Box。
- Stanford Bunny。
- 简单室内 PBR 场景。

指标：

- 固定 SPP 下耗时。
- 固定时间下 PSNR / SSIM。
- BVH build time。
- ray / second。
- intersection / second。

参考对象：

- Blender Cycles 作为质量 reference。
- Embree/OptiX 作为性能量级参考，不直接要求同架构。

### 7.3 启动与资源 benchmark

目标：验证 Asset Manager、Shader 缓存、资源复用是否有效。

指标：

- 首次加载时间。
- 二次加载时间。
- Mesh 解析次数。
- Shader 编译次数。
- GPU texture 创建次数。
- 资源数量统计。

验收原则：

- 每个优化任务必须在 benchmark 中至少改善一个指标，或明确说明它是为了后续功能铺路。
- 没有测量手段的性能优化不进入 P0。

## 八、近期执行顺序

当前推荐顺序：

1. 收敛 `asset-manager-design.md`，只保留 Phase 1 同步缓存与统计设计。
2. 实现 Asset Manager 同步版，并接入 Texture / UMesh / Shader 的真实加载路径。
3. 写最小资源 benchmark：同一资源重复加载两次，验证缓存命中和 stats。
4. 实现 GPU/CPU profiling 基础。
5. 基于 profiling 做视锥体剔除和 UBO 脏标记。
6. 开始 IBL 功能主线。

这个顺序的核心逻辑是：先治理资源，再建立测量，再做性能优化，最后补功能。这样每一步都能对当前项目产生实际价值，也不会把大型引擎的复杂架构提前搬进来。
