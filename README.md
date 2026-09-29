# Genius Invokation Virtual Machine

GIVM 是一个使用 C++23 编写的非官方领域虚拟机，面向七圣召唤及其衍生的卡牌游戏。不同前端可以向它提供定义源和 command 序列；它负责依赖解析、编译、链接和确定性解释执行，并提供可复制的对局状态与公开观察接口。

编译时通过 `compile_mode` 选择普通模式或包含额外观察暂停的模式。`executor::start(library, table)` 返回初始化视图；输入视图提交选择时直接推进，初始化与观察视图通过 `resume(library, table, random)` 继续，返回的状态用于获取下一个视图。

## 使用

通过 `add_subdirectory` 或 `FetchContent` 将本仓库加入 CMake 工程，然后链接静态库目标：

```cmake
target_link_libraries(your_target PRIVATE givm::givm)
```

本库需要编译并链接，不能只复制头文件使用。使用方与 GIVM 必须保持 `NDEBUG` 定义一致；调试检查会影响部分公开类型的布局，不能混用调试与发布配置的二进制。

完整的公开接口可以通过以下头文件引入：

```cpp
#include <givm/givm.hpp>
```

也可以单独引入 `givm/` 下的接口，例如 `<givm/definition.hpp>`、`<givm/table.hpp>` 和 `<givm/executor.hpp>`。

公开接口见[参考手册](docs/zh/reference.md)。设计取舍、历史方案和源码问题见[开发备忘](docs/zh/notes.md)。
