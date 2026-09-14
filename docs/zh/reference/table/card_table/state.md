[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **state**

# givm::card_table::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const table_state& state() const noexcept;
```

访问双方共同的对局状态，例如当前回合和当前行动玩家。

## 返回值

牌桌中的 [`table_state`](../table_state.md) 的只读引用。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::card_table table{};
    std::println("初始回合数: {}", table.state().round_number);
    std::println("初始行动玩家: {}", table.state().active_player.index);
}
```

输出

```text
初始回合数: 0
初始行动玩家: 0
```
