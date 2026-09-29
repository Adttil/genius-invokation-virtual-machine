# Genius Invokation Virtual Machine

GIVM 是一个使用 C++23 编写的非官方领域虚拟机，面向七圣召唤及其衍生的卡牌游戏。不同前端可以向它提供定义源和 command 序列；它负责依赖解析、编译、链接和确定性解释执行，并提供可复制的对局状态与公开观察接口。

编译时通过 `compile_mode` 选择普通模式或包含额外观察暂停的模式。`executor::start(library, table)` 返回初始化视图；输入视图提交选择时直接推进，初始化与观察视图通过 `resume(library, table, random)` 继续，返回的状态用于获取下一个视图。

## 使用

通过 `add_subdirectory` 或 `FetchContent` 将本仓库加入 CMake 工程，然后链接静态库目标：

```cmake
target_link_libraries(your_target PRIVATE givm::givm)
```

本库需要编译并链接，不能只复制头文件使用。使用方与 GIVM 必须保持 `NDEBUG` 定义一致；调试检查会影响部分公开类型的布局，不能混用调试与发布配置的二进制。

按使用场景选择公开聚合头：

| 头文件 | 用途 |
| --- | --- |
| `<givm/source_library.hpp>` | 持有、登记、查询和合并定义源库，适合声明源库工厂的公共头 |
| `<givm/source.hpp>` | 编写定义源，使用命令、事件、编译上下文及响应上下文 |
| `<givm/compile.hpp>` | 将源库和对局流程编译为定义库，处理编译结果与诊断 |
| `<givm/runtime.hpp>` | 使用编译后的定义库、牌桌、执行器和视图推进游戏 |
| `<givm/givm.hpp>` | 全部公开能力及官方基础定义 |

`source_library.hpp` 提供完整的 `definition_source_library`。内容库的公共头包含它后，调用方链接相应库即可接收、复制、移动、合并和查询工厂返回的源库，无须额外包含定义源的实现。

只使用牌桌时仍可包含 `<givm/table.hpp>`。原有 `<givm/definition.hpp>`、`<givm/executor.hpp>` 保留完整模块接口。官方基础定义也可单独通过 `<givm/basic_definitions.hpp>` 引入。

公开接口见[参考手册](docs/zh/reference.md)。设计取舍、历史方案和源码问题见[开发备忘](docs/zh/notes.md)。
