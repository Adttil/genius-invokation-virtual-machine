[givm](../../../reference.md) / [定义](../../definition.md) / [definition_library](../definition_library.md) / **entry**

# givm::definition_library::entry

定义于头文件 `<givm/definition.hpp>`

```cpp
static constexpr execution_position entry() noexcept;
```

取得开始一场对局时首先执行的指令位置。通常由 [`executor::enter_entry`](../../executor/executor/enter_entry.md) 使用；也可用于检查编译所得对局流程的开头。

## 返回值

对局流程的初始 [`execution_position`](../execution_position.md)。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{} }
    );
    std::println("首先洗牌: {}", library.instruction(library.entry()).is<givm::shuffle_deck>());
}
```

输出

```text
首先洗牌: true
```
