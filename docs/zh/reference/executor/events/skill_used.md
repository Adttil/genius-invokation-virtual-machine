[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **skill_used**

# givm::skill_used

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct skill_used;
```

技能使用完成后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次使用的技能标识；只读 |
| `kind` | `const skill_kind` | 技能种类；只读 |
| `speed` | [`const action_speed`](../../enums/action_speed.md) | 行动速度；只读 |
| `effect_cancelled` | `const bool` | 本次技能效果是否已被取消；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::skill_used event{ .skill = {}, .kind = givm::skill_kind::elemental_skill, .speed = givm::action_speed::combat, .effect_cancelled = false };
    std::println("元素战技: {}", event.kind == givm::skill_kind::elemental_skill);
    std::println("效果已取消: {}", event.effect_cancelled);
}
```

输出

```text
元素战技: true
效果已取消: false
```
