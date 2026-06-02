# 240 FPS 动画 GPU 占用测试

本文记录同一台 Windows 机器上，使用 EUI DSL 动画写法运行 `animation_240fps` 测试程序时，不同窗口后端和渲染后端的 GPU 占用、FPS 与进程内存占用。

本次重新测试没有沿用旧的低层渲染探针，也没有追加之前的错误数据。测试程序走 `compose()`、`ui.stack().onFrame(...)`、`ui.rect(...)`、`ui.text(...)` 这条真实 EUI DSL 路径。

## 2026-06-02 Windows 重新采样

环境：

- 系统：Microsoft Windows 10 企业版 LTSC 10.0.19044，64-bit
- CPU：12th Gen Intel(R) Core(TM) i7-12650H，10 核 16 线程
- GPU：NVIDIA GeForce RTX 4060 Laptop GPU，Driver 32.0.15.9621
- 屏幕：2560x1600，240 Hz
- CMake：4.3.1
- 分支：`vk`
- 代码基线：`20beb28`
- 构建类型：Release
- 测试窗口：1280x720
- 目标帧率：240 FPS

测试内容：

- 96 个矩形持续进行平移、旋转、X/Y 轴旋转、缩放、圆角、颜色、透明度、边框和阴影变化。
- 11 行文字持续进行颜色、字号、平移、缩放和旋转变化。
- 每帧由 `onFrame(float deltaSeconds)` 推进累计时间，画面状态由连续时间函数直接决定。
- 不使用 transition 补间包裹每帧目标值，避免每帧重置补间起点导致动画追帧和测试结果失真。
- 窗口标题开启 FPS 显示，用于记录实际渲染帧率。

构建命令：

```powershell
cmake --build --preset opengl-glfw-release --target animation_240fps --parallel
cmake --build --preset opengl-sdl2-release --target animation_240fps --parallel
cmake --build --preset vulkan-glfw-release --target animation_240fps --parallel
cmake --build --preset vulkan-sdl2-release --target animation_240fps --parallel
```

采样方法：

- 每个 preset 单独启动 `build/<preset>/Release/animation_240fps.exe`。
- 启动后预热 4 秒。
- 使用 Windows `Get-Counter '\GPU Engine(*)\Utilization Percentage'` 按进程 PID 汇总 GPU Engine 占用。
- 每组采样 10 次，每次间隔 1 秒。
- 同时记录窗口标题 FPS、Working Set 和 Private Memory。
- 每组测试结束后强制关闭对应测试进程，避免残留进程影响下一组。

结果：

| Preset | 实际 FPS 平均值 | FPS 范围 | GPU 平均占用 | GPU 范围 | Working Set 平均值 | Working Set 范围 | Private Memory 平均值 | Private Memory 范围 |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `opengl-glfw-release` | 105.4 | 103-107 | 6.00% | 5.51-6.35% | 81.3 MiB | 81.1-82.0 MiB | 93.0 MiB | 92.8-93.7 MiB |
| `opengl-sdl2-release` | 237.9 | 235-240 | 10.14% | 8.00-11.33% | 78.7 MiB | 78.6-79.5 MiB | 91.3 MiB | 91.2-92.1 MiB |
| `vulkan-glfw-release` | 230.2 | 224-234 | 14.06% | 5.92-20.47% | 126.7 MiB | 124.6-133.7 MiB | 149.1 MiB | 146.9-156.1 MiB |
| `vulkan-sdl2-release` | 237.7 | 229-240 | 13.85% | 6.44-19.21% | 126.4 MiB | 124.8-130.8 MiB | 147.6 MiB | 146.0-152.0 MiB |

GPU Engine 平均组成：

| Preset | Engine 平均值 |
| --- | --- |
| `opengl-glfw-release` | `3d:6.00; copy:0.00; legacyoverlay:0.00; ofa_0:0.00; security:0.00; videodecode:0.00; videoencode:0.00; vr:0.00` |
| `opengl-sdl2-release` | `3d:10.14; copy:0.00; legacyoverlay:0.00; ofa_0:0.00; security:0.00; videodecode:0.00; videoencode:0.00; vr:0.00` |
| `vulkan-glfw-release` | `3d:14.06; copy:0.00; legacyoverlay:0.00; ofa_0:0.00; security:0.00; videodecode:0.00; videoencode:0.00; vr:0.00` |
| `vulkan-sdl2-release` | `3d:13.85; copy:0.00; legacyoverlay:0.00; ofa_0:0.00; security:0.00; videodecode:0.00; videoencode:0.00; vr:0.00` |

## 2026-06-02 OpenGL 状态缓存优化后复测

本轮复测在同一台机器、同一 `animation_240fps` 测试程序和同一采样方法下进行，只复测 OpenGL 后端。变更内容是减少 OpenGL 后端重复提交的渲染状态，包括 program、VAO、array buffer、texture unit、texture、blend 和 scissor 状态。

结果：

| Preset | 实际 FPS 平均值 | FPS 范围 | GPU 平均占用 | Working Set 平均值 |
| --- | ---: | ---: | ---: | ---: |
| `opengl-glfw-release` | 218.0 | 212-224 | 10.79% | 83.0 MiB |
| `opengl-sdl2-release` | 238.9 | 237-240 | 13.80% | 78.7 MiB |

对比结论：

- `opengl-glfw-release` 从约 105.4 FPS 提升到约 218.0 FPS，说明之前 GLFW/OpenGL 在该测试里有明显的重复状态提交开销。
- `opengl-sdl2-release` 仍然接近 240 FPS 上限，帧率基本不变；GPU 占用升高主要来自实际提交帧率更稳定地贴近 240 FPS，以及采样窗口内负载波动，不能直接理解为单帧成本升高。
- 本轮优化不改变测试画面、目标帧率和交互路径，只减少后端对驱动的重复状态调用。

## 结论

- 本次有效的 240 FPS 对比主要看 `opengl-sdl2-release`、`vulkan-glfw-release`、`vulkan-sdl2-release` 三组。它们实际 FPS 都接近 240。
- 在接近 240 FPS 的同一 DSL 动画负载下，Vulkan 当前 GPU 占用约 13.85-14.06%，OpenGL/SDL2 约 10.14%。这说明当前 Windows + RTX 4060 Laptop 环境里，EUI Vulkan 后端在这类大量小矩形、阴影、圆角和文字动画场景下还没有比 OpenGL 更省 GPU。
- Vulkan 的 Working Set 比 OpenGL/SDL2 高约 47-48 MiB，Private Memory 高约 56-58 MiB。这部分差异符合 Vulkan 后端显式持有 swapchain、render cache、descriptor/image pool、pipeline、command buffer、upload ring 等对象后的进程内存表现；后续需要继续拆分到 Vulkan 资源统计，不能只看任务管理器总数下结论。
- `opengl-glfw-release` 本轮实际只有约 105 FPS，不能直接拿 6.00% GPU 和 240 FPS 组硬比。OpenGL 后端已经调用 `glfwSwapInterval(0)`，本轮低帧率更像 GLFW/OpenGL 在当前 Windows/DWM/驱动组合下的实际调度结果，需要单独定位。

## 动画只动一次的原因

`onFrame` 在 runtime 中会设置 `needsCompose_`、`needsRender_` 和 `animating_`，持续调度本身是通的。之前测试看起来像只动一次，根因是测试程序把连续变化的每帧目标值又套了一层 transition。这样每帧都会把新的目标传给 `AnimatedValue::setTarget()`，补间起点不断重置，视觉上会变成追逐目标、迟滞或某些属性像只完成了一次。

当前测试程序改为由 `elapsed` 连续时间直接决定当前帧状态，不再对每帧目标值套 transition，因此画面会持续运动，且采样更接近真实“每帧更新 UI 状态”的 EUI DSL 动画负载。

## 后续定位点

- 给 Vulkan 后端增加资源统计输出：swapchain image、render cache image、backdrop image、descriptor pool/image pool、upload ring、pipeline/cache、text atlas/image texture 分别记录数量和估算字节数。
- 单独定位 OpenGL/GLFW 在本机 240 Hz 下只能跑到约 105 FPS 的原因，避免后续把不同实际 FPS 的样本放在同一基准里比较。
- 增加一组关闭阴影或降低文字变化的对照测试，用来拆分圆角阴影 shader、文本 atlas 更新、脏区合并对 GPU 占用的贡献。
