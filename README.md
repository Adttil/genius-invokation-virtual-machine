# Genius Invokation Virtual Machine

GIVM 是一个使用 C++23 编写的非官方领域虚拟机，面向七圣召唤及其衍生的卡牌游戏。不同前端可以向它提供定义源和指令程序；它负责依赖解析、编译、链接和确定性解释执行，并提供可复制的对局状态与公开观察接口。

## 使用

通过 `add_subdirectory` 或 `FetchContent` 将本仓库加入 CMake 工程，然后链接这个纯头文件库目标：

```cmake
target_link_libraries(your_target PRIVATE givm::givm)
```

完整的公开接口可以通过以下头文件引入：

```cpp
#include <givm/givm.hpp>
```

也可以单独引入 `givm/` 下的接口，例如 `<givm/definition.hpp>`、`<givm/table.hpp>` 和 `<givm/executor.hpp>`。

公开行为与规则拓展协议见[项目文档](docs/README.md)。
