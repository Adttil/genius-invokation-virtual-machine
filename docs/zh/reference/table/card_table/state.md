[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **state**

# givm::card_table::state

定义于头文件 `<givm/table.hpp>`

```cpp
template<class Self>
constexpr auto& state(this Self& self) noexcept;
```

访问双方共同的对局状态，例如当前回合和当前行动玩家。

## 模板参数

|  |  |
| --- | --- |
| `Self` | 由牌桌对象推导并保留其 const 限定的类型 |

## 参数

|  |  |
| --- | --- |
| `self` | 要访问的牌桌左值 |

## 返回值

牌桌中的 [`table_state`](../table_state.md) 引用；只读牌桌返回只读引用。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    table.state().round_number = 2;
    table.state().active_player = givm::player_id{ 1 };
    std::println("当前回合: {}", table.state().round_number);
    std::println("行动玩家: {}", table.state().active_player.index);
}
```

输出

```text
当前回合: 2
行动玩家: 1
```
