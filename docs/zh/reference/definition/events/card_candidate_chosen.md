[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **card_candidate_chosen**

# givm::card_candidate_chosen

定义于头文件 `<givm/definition.hpp>`

```cpp
struct card_candidate_chosen;
```

候选牌定义选定后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `const player_id` | 这次事件对应的玩家；只读 |
| `definition_id` | `const definition_id<card_definition>` | 对应的牌定义标识；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::card_candidate_chosen event{ .player = givm::player_id{ 1 }, .definition_id = {} };
    std::println("由玩家 1 选择: {}", event.player == givm::player_id{ 1 });
}
```

输出

```text
由玩家 1 选择: true
```
