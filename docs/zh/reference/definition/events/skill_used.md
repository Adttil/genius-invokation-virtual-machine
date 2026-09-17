[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **skill_used**

# givm::skill_used

定义于头文件 `<givm/definition.hpp>`

```cpp
struct skill_used;
```

技能使用完成后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次使用的技能标识；只读 |
| `targets` | `const std::array<skill_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略 |
| `speed` | [`const action_speed`](../../enums/action_speed.md) | 行动速度；只读 |
| `effect_cancelled` | `const bool` | 本次技能效果是否已被取消；只读 |

## 注意

本次使用的技能及目标与确认的行动一致。无论技能原效果是否被取消，本事件都会在原效果结算之后广播；`effect_cancelled` 保留取消结果。技能分类可通过该技能的定义标签查询。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::skill_used event{ .skill = {}, .targets = {}, .speed = givm::action_speed::combat, .effect_cancelled = false };
    std::println("效果已取消: {}", event.effect_cancelled);
}
```

输出

```text
效果已取消: false
```
