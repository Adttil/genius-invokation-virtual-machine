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
| `speed` | [`const action_speed`](../../enums/action_speed.md) | `skill_will_be_used` 最终确定的速度；独立 `use_skill` 命令不据此改变外层行动速度；只读 |
| `effect_cancelled` | `const bool` | 本次技能效果是否已被取消；只读 |
| `flags` | `const skill_flags` | 本次使用的技能性质；效果与前置通知沿用同一结果 |

## 注意

本次使用的技能、目标及性质与前置通知一致。无论技能原效果是否被取消，本事件都会在原效果结算之后广播；`effect_cancelled` 保留取消结果。

[`use_skill`](../commands/use_skill.md) 命令会等待本事件的所有响应程序完成后才返回外层效果。因此，天赋牌效果通过该命令使用技能时，本通知及其响应先于该牌的 [`card_played`](card_played.md)。独立命令不读取技能费用，也不根据 `speed` 改变外层行动速度。

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

行动选择中，普通攻击的重击和下落攻击性质在行动候选建立时确定，费用响应可以据此减费。`use_skill` 固定模式仅使用技能标签确定的固有分类，动态模式使用输入的 `flags`，不会自动追加这两种性质。定义通过 `event.flags.to_damage_flags()` 将相应性质显式写入动态伤害输入；不会自动修改其他命令的伤害。
