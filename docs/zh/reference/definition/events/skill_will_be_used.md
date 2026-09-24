[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **skill_will_be_used**

# givm::skill_will_be_used

定义于头文件 `<givm/definition.hpp>`

```cpp
struct skill_will_be_used;
```

技能效果生效前的事件。响应者可以调整行动速度或取消技能效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `skill` | `const skill_id` | 本次使用的技能标识；只读 |
| `targets` | `const std::array<skill_target_id, 2>` | 本次采用的两个目标位置；只读，未使用的位置忽略 |
| `speed` | [`action_speed`](../../enums/action_speed.md) | 行动选择时为本次行动速度；独立 `use_skill` 命令初始为 `fast`，不改变外层行动速度 |
| `effect_cancelled` | `bool` | 是否取消技能效果，初始为 false |
| `flags` | `const skill_flags` | 本次使用的技能性质；效果与通知沿用同一结果 |

## 注意

行动选择在费用结算完成后广播本事件；[`use_skill`](../commands/use_skill.md) 命令直接广播，不计算或支付技能费用。响应者可以将 `effect_cancelled` 设为 `true`，跳过技能自身的 [`skill_effect`](skill_effect.md)；这不会退还费用或取消之后的 [`skill_used`](skill_used.md) 广播。

独立命令保留响应修改后的 `speed` 并传给完成通知，但不据此修改外层行动速度、当前行动玩家或下落攻击机会。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::skill_will_be_used event{ .skill = {}, .targets = {}, .speed = givm::action_speed::combat, .effect_cancelled = false };
    event.effect_cancelled = true;
    std::println("效果已取消: {}", event.effect_cancelled);
}
```

输出

```text
效果已取消: true
```

行动选择中，普通攻击的重击和下落攻击性质在行动候选建立时确定，费用响应可以据此减费。`use_skill` 固定模式仅使用技能标签确定的固有分类，动态模式使用输入的 `flags`，不会自动追加这两种性质。定义通过 `event.flags.to_damage_flags()` 将相应性质显式写入动态伤害输入；不会自动修改其他命令的伤害。
