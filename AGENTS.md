# AGENTS.md

## 构建命令

```bash
# 配置（默认全部开启）
cmake -B build -S .

# 选择性开启后端
cmake -B build -S . \
  -DEDA_ENABLE_HTTP=ON -DEDA_HTTP_BACKEND_FAKE=ON -DEDA_HTTP_BACKEND_MONGOOSE=OFF \
  -DEDA_ENABLE_MODBUS_MASTER=ON -DEDA_MODBUS_MASTER_BACKEND_FAKE=ON

# 编译
cmake --build build

# 运行
./build/eda_demo http      # HTTP 服务端 demo（默认）
./build/eda_demo modbus    # Modbus Master demo
./build/eda_demo timer     # 定时器 + HTTP 客户端 demo
```

## 关键注意事项

- **生成文件**：`core/bootstrap/BuildFeatures.h.in` 由 CMake `configure_file()` 生成到 `build/generated/bootstrap/BuildFeatures.h`，把 CMake option 映射为 `build_features::` 命名空间下的 `constexpr bool`。修改 CMake option 后必须重新 cmake 配置。
- **OpenCode 配置**：`.vscode/settings.json` 已配置 clangd 的 `--compile-commands-dir`，修改编译选项后需 `cmake -B build` 重新生成 `compile_commands.json`，然后重启 clangd。
- **全部头文件使用 `#pragma once`**（非 include guard）。

## 架构

```
demo/  →  消费框架，提供具体的 IBusinessModule / IHttpRouteProvider 实现
core/  →  可复用框架：启动、事件总线、传输层、领域模型
```

- **核心模式**：`ApplicationBootstrap::create()` 作为组合根，通过 `AssemblyContext` 串联各 `*Assembly::install()` 步骤，最终产出 `ApplicationHost`。
- **后端可插拔**：每个协议（HTTP Server / HTTP Client / Modbus Master）都有 **Fake** 和 **Real** 两套后端，通过 CMake option 编译期选择 + 运行时 `AppConfig.backend` 枚举匹配。编译期开关与运行时配置不一致时 `AppConfigValidation` 会报错。
- **新增协议后端流程**：加 CMake option → `BuildFeatures.h.in` 加宏 → 实现 `I*Adapter` → `*BackendFactory` 加分支 → 可选加 `*Assembly`。

## 关键抽象（ABC）

| 接口 | 所在文件 | 用途 |
|------|----------|------|
| `IBusinessModule` | `core/application/modules/IBusinessModule.h` | 用户业务逻辑扩展点，`install(EventBus&)` |
| `IHttpRouteProvider` | `core/interfaces/http/IHttpRouteProvider.h` | HTTP 路由定义扩展点 |
| `IHttpServerAdapter` | `core/infrastructure/transport/http/IHttpServerAdapter.h` | HTTP 服务端后端 |
| `IHttpClientAdapter` | `core/infrastructure/transport/http/IHttpClientAdapter.h` | HTTP 客户端后端 |
| `IModbusMasterAdapter` | `core/infrastructure/transport/modbus/IModbusMasterAdapter.h` | Modbus Master 后端 |

## 事件系统

事件定义在 `core/domain/events/DomainEvent.h`。内置事件在 `FrameworkEvents` 命名空间。添加自定义事件：
1. 定义 payload 结构体（放在 `core/domain/model/EventData.h`）
2. 定义 `TypedEvent<Payload>` 实例（参考 `demo/timer_test/TimerDemoEvents.h`）
3. 在 `IBusinessModule::install()` 中 `bus.subscribe(event, handler)`

## 第三方依赖

- **mongoose**（vendored）：`third_party/mongoose/mongoose.c` + `.h`，v7.20。仅在 HTTP mongoose 后端开启时编译进项目。
- **libmodbus**（vendored）：预编译的 `third_party/libmodbus/lib/libmodbus.a` + 头文件。仅在 Modbus libmodbus 后端开启时链接。

## 无正式测试

项目没有单元测试文件。Fake 后端（`FakeHttpServerAdapter`、`FakeHttpClientAdapter`、`FakeModbusMasterAdapter`）可作为测试替身。三个 demo 程序充当集成验证。
