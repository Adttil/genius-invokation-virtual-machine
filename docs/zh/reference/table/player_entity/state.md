[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **state**

# givm::player_entity::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto& state() const;
```

访问该玩家在对局中的当前状态。

## 返回值

实体持有的 [`player_state`](../player_state.md) 引用；可写实体返回可写引用，只读视图返回只读引用。

## 注意

直接修改状态不执行规则结算或事件广播。

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
    const auto entity = table[givm::player_id{ 0 }];
    entity.state().dice[givm::elemental_dice::omni] = 2;
    std::println("骰子数: {}", entity.state().dice.total());
}
```

输出

```text
骰子数: 2
```
