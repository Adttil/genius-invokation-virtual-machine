[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **(构造函数)**

# givm::card_table::(构造函数)

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr card_table(
    const definition_library_type& definition_library,
    game_parameters parameters = {}
);
```

准备一张尚未装载牌组的牌桌，使用指定的对局参数。

双方玩家已经存在；角色和卡牌等实体尚未加入。

## 参数

|  |  |
| --- | --- |
| [`definition_library`](../../definition/definition_library.md) | 为这场对局提供实体定义的定义库 |
| `parameters` | 对局参数，省略时使用 [`game_parameters`](../game_parameters.md) 的默认值 |

## 返回值

（无）

## 注意

定义库须比牌桌及其副本存活更久。牌桌副本拥有独立的对局状态，并继续使用同一份定义库。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    givm::card_table configured{ library, { .hand_limit = 12 } };
    std::println("手牌上限: {}", configured.parameters().hand_limit);
    std::println("初始回合数: {}", configured.state().round_number);
}
```

输出

```text
手牌上限: 12
初始回合数: 0
```
