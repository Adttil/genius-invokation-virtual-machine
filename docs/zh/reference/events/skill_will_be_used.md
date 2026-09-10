[givm](../../reference.md) / [事件](../events.md) / **skill_will_be_used**

# givm::skill_will_be_used

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct skill_will_be_used;
```

角色即将使用技能。响应者可以调整行动速度或取消技能效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次使用的技能标识；只读 |
| `kind` | `const skill_kind` | 技能种类；只读 |
| `speed` | [`action_speed`](../enums/action_speed.md) | 行动速度 |
| `effect_cancelled` | `bool` | 是否取消技能效果，初始为 false |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::skill_will_be_used event{ .skill = {}, .kind = givm::skill_kind::elemental_skill, .speed = givm::action_speed::combat, .effect_cancelled = false };
    event.effect_cancelled = true;
    std::println("元素战技: {}", event.kind == givm::skill_kind::elemental_skill);
    std::println("效果已取消: {}", event.effect_cancelled);
}
```

输出

```text
元素战技: true
效果已取消: true
```
