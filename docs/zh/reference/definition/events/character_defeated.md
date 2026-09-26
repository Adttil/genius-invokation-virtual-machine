[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **character_defeated**

# givm::character_defeated

定义于头文件 `<givm/definition.hpp>`

```cpp
struct character_defeated;
```

角色被击倒后的通知。

伤害使角色生命降至零时，先完成 [`character_will_be_defeated`](character_will_be_defeated.md) 的全部响应。若生命仍为零且对局尚未结束，清除其全部 attachment、充能和元素附着后，全局广播本事件；被清除的 attachment 不参与本次广播。全部响应结束后，才继续本段伤害的默认反应实体生成及后续伤害。

复活成功或击倒导致立即终局时，不广播本事件。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 已被击倒的角色；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::character_defeated event{ .target = { .player_id = givm::player_id{ 0 }, .index = 1 } };
    std::println("被击倒的角色序号: {}", event.target.index);
}
```

输出

```text
被击倒的角色序号: 1
```
