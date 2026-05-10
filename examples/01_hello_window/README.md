# Chapter 01 — Hello, Engine

> **本章定位**:打开引擎大门,跑出第一个属于你自己的引擎窗口。

---

## 本章目标
1. 用 SDL3 创建一个 1280×720 的窗口
2. 创建 OpenGL 4.5 Core 上下文,glad 加载函数
3. 主循环:轮询事件 → 清屏(深蓝色)→ SwapBuffers
4. ESC 键退出
5. 窗口标题实时显示 FPS

## 跑出来的 Demo
`build/Release/chapter_01_demo.exe` — 双击启动,可看到一个稳定 60fps 的深蓝窗口,标题栏显示 `Hello, Engine — 60.0 FPS`。按 ESC 退出。

## 学到的前沿技术
- **SDL3** vs SDL2 / GLFW 的区别(SDL3 是 2024-2025 才稳定的新一代,事件 API 重写,更适合游戏)
- **OpenGL 4.5 Core Profile**:为什么不是 Compatibility?为什么不是 ES?为什么不是 Vulkan?
- **glad2 加载器**:函数指针怎么解析的,为什么不能直接 `#include <GL/gl.h>`
- **VSync / 双缓冲 / Tearing**:为什么不开 VSync 会画面撕裂
- **帧率独立**:为什么所有运动都要乘以 `deltaTime`

## 背景知识(为什么这章必要)
现代游戏引擎都把"窗口 + 输入 + 渲染上下文"作为最底层的抽象。Unity 内部用 SDL,Unreal 内部用自己的 RHI,Godot 用自己的窗口层。**这层做不好,后面所有渲染都白搭**(典型问题:不同显卡兼容性差、帧率跳跃、ALT+TAB 黑屏)。

SDL3 是 2025 年游戏开发界的"事实标准"——它是 Valve / Epic 都在用的跨平台基础库,API 简洁,跨 Win/Mac/Linux/Mobile/主机。我们选 SDL3 而非 GLFW,因为 SDL3 还自带音频 / 手柄 / 触屏 / 高 DPI 支持,后面所有章节都能复用它。

## 架构设计

```
App
 ├─ Window (SDL_Window + SDL_GLContext)
 ├─ (后续章节加 Input/Time/World/...)
 └─ 主循环
     ├─ PollEvents()       ← SDL 事件 → 内部状态
     ├─ Time::Tick()       ← 计算 deltaTime / FPS
     ├─ glClear            ← 深蓝色背景
     └─ SwapBuffers()      ← 翻页
```

## 涉及源文件
- [`engine/core/Window.h/cpp`](../../engine/core/Window.h) — SDL3 + OpenGL 上下文封装
- [`engine/core/App.h/cpp`](../../engine/core/App.h) — 主循环骨架
- [`depends/glad/`](../../depends/glad/) — OpenGL 函数加载器
- [`examples/01_hello_window/main.cpp`](main.cpp) — 本章 demo

## 实施清单(checklist)
- [x] SDL3 通过 CMake FetchContent 下载
- [x] glad2 集成
- [x] Window 类封装
- [x] App::Run 主循环
- [ ] CMakeLists.txt 顶层配置
- [ ] chapter_01_demo target
- [ ] FPS 标题更新
- [ ] 截图录入 README

## 常见坑(实施时再填具体)
- SDL3 在 Windows 上可能链接 `SDL3.dll` 而非 `SDL3-static`,要确认 CMake 选项
- glad 加载必须在 `SDL_GL_MakeCurrent` 之后调用
- ESC 退出要用 `SDL_EVENT_KEY_DOWN` + `SDL_SCANCODE_ESCAPE`,不是 SDLK 键码

## 延伸阅读
- [SDL3 Migration Guide](https://wiki.libsdl.org/SDL3/README/migration)
- [OpenGL Programming Guide(红宝书)](https://www.opengl-redbook.com/) — 第 1-2 章
- [LearnOpenGL — Hello Window](https://learnopengl.com/Getting-started/Hello-Window) — 经典入门
- [glad GitHub](https://github.com/Dav1dde/glad)

## 下一章预告
**Ch 02 — ECS + Time + Input**:有了窗口,我们就能把"实体+组件"机制搭起来,准备承载未来所有的游戏对象。
