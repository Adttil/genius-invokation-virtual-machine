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
| `speed` | [`action_speed`](../../enums/action_speed.md) | 行动速度 |
| `effect_cancelled` | `bool` | 是否取消技能效果，初始为 false |
| `flags` | `const skill_flags` | 本次行动的技能性质；在支付前报价时已确定，效果与通知沿用同一结果 |

## 注意

费用结算完成后广播本事件。响应者可以将 `effect_cancelled` 设为 `true`，跳过技能自身的 [`skill_effect`](skill_effect.md)；这不会退还费用或取消之后的 [`skill_used`](skill_used.md) 广播。技能分类通过定义标签查询。

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

普通攻击的重击和下落攻击性质在行动候选建立时确定，费用响应可以据此减费。定义通过 `event.flags.to_damage_flags()` 将相应性质显式写入动态伤害输入；不会自动修改其他命令的伤害。
