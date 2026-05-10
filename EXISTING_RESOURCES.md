# AIForge — 已有资源说明

## 来自 Mini Game Engine 教程项目的可复用资源

原项目路径：E:\git_project\games\engine\mini_game_engine

### 已有的库（可直接复制depends目录）

| 库 | 位置 | 说明 |
|------|------|------|
| GLFW | mini_game_engine/depends/glfw-3.3-3.4/ | Phase 1用SDL3替代，但保留备用 |
| Dear ImGui | mini_game_engine/depends/imgui/ | 直接复用 |
| stb_image | mini_game_engine/depends/stb/ | 直接复用 |
| miniaudio | mini_game_engine/depends/miniaudio/ | 被FMOD替代，备用 |
| Assimp | mini_game_engine/depends/assimp/ | 直接复用（头文件+lib+dll） |
| FMOD | mini_game_engine/depends/fmod/ | 直接复用（Core+Studio） |
| PhysX | mini_game_engine/depends/physx/ | 直接复用（通过CMakeLists.txt.Physx自动下载） |
| OpenGL | 通过glad/gl.h（在GLFW的deps目录下） | 直接复用 |

### 没有的库（需要下载）

| 库 | 获取方式 | 用途 |
|------|----------|------|
| SDL3 | CMake FetchContent自动下载 | 窗口/输入/手柄 |
| Vulkan | Phase 5再下载，暂不需要 | 可选渲染后端 |

### SDL3获取方式（CMake自动下载）

```cmake
include(FetchContent)
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.0)
FetchContent_MakeAvailable(SDL3)
target_link_libraries(${TARGET_NAME} SDL3::SDL3)
```

如果网络下载失败，可以手动：
1. 从 https://github.com/libsdl-org/SDL/releases 下载SDL3源码zip
2. 解压到 depends/SDL3/
3. CMake中改为 add_subdirectory(depends/SDL3)

### 已有的素材资源

| 类型 | 位置 | 内容 |
|------|------|------|
| 3D模型 | mini_game_engine/data/models/fbx/ | X Bot + 12个Mixamo动画 |
| 敌人模型 | mini_game_engine/data/models/fbx/enemies/ | Mutant + Zombie（各5个FBX） |
| OBJ模型 | mini_game_engine/data/models/obj/ | StopSign, minecraft |
| PBR材质 | mini_game_engine/data/texture/materials/ | 5组（bricks/wood/metal/metalplates/woodfloor） |
| 3D音效 | mini_game_engine/data/audio/sfx3d/ | 10个（hit/hurt/slash/dash/jump等） |
| BGM | mini_game_engine/data/audio/bgm/ | 4首 |
| Shader | mini_game_engine/data/shader/ | basic3d/phong/material/skinning/skybox/particle |
| 2D素材 | mini_game_engine/data/texture/ | 角色/背景/特效/瓦片 |

### 已有的引擎代码（可参考但AIForge要重写）

| 模块 | 文件 | 可参考的设计 |
|------|------|-------------|
| 数学库 | source/math/Vec2/3/4.h, Mat3/4.h | 向量矩阵运算 |
| 3D网格 | source/engine3d/Mesh3D.h/cpp | Vertex3D结构、VAO/VBO/EBO |
| 模型加载 | source/engine3d/ModelLoader.h | Assimp集成方式 |
| 纹理 | source/engine3d/Texture3D.h | stb_image加载+Material3D |
| 摄像机 | source/engine3d/Camera3D.h | Orbit/FPS/TPS三模式 |
| 角色控制 | source/engine3d/CharacterController3D.h | 加速度移动/跳跃/冲刺 |
| 骨骼动画 | source/engine3d/Animator.h | 骨骼树/蒙皮/动画插值 |
| 物理 | source/engine3d/PhysicsWorld.h | PhysX封装 |
| 音频 | source/engine3d/AudioSystem3D.h | FMOD封装 |
| Shader | source/engine/Shader.h | LoadFromFile/Bind/SetMat4等 |

### AIForge项目建议的目录结构

```
E:\git_project\games\engine\AIForge\     ← 新项目，和mini_game_engine同级
├── engine/                               ← 引擎源码（全新编写）
├── data/                                 ← 可从mini_game_engine/data/复制素材
├── depends/                              ← 可从mini_game_engine/depends/复制库
│   ├── imgui/                            ← 复制
│   ├── stb/                              ← 复制
│   ├── assimp/                           ← 复制
│   ├── fmod/                             ← 复制
│   └── SDL3/                             ← FetchContent下载或手动下载
├── examples/
├── templates/
├── tools/
├── docs/
├── CMakeLists.txt
└── README.md
```
