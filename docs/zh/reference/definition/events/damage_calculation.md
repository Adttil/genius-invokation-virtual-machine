[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_calculation**

# givm::damage_calculation

定义于头文件 `<givm/definition.hpp>`

```cpp
struct damage_calculation;
```

伤害计算事件。响应者可以调整来源、目标、基础伤害、倍率和伤害种类，决定接下来怎样结算这次伤害。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`damage_source_id`](damage_source_id.md) | 本次伤害的来源 |
| `target` | [`character_id`](../../table/character_id.md) | 受到本次伤害的角色，可由响应者调整 |
| `value` | `std::uint32_t` | 应用伤害倍率前的伤害值 |
| `multiplier_numerator` | `std::uint16_t` | 伤害倍率的分子，初始为 1 |
| `multiplier_denominator` | `std::uint16_t` | 伤害倍率的非零分母，初始为 1 |
| `type` | [`damage_type`](../../enums/damage_type.md) | 伤害种类 |
| `flags` | [`damage_flags`](../../enums/damage_flags.md) | 伤害附加属性 |
| `already_handled_reaction` | `bool` | 是否已经处理伤害计算中的元素反应加成，初始为 false |

## 注意

本事件结束后，才按最终 `target`、`type` 和目标当前附着确定元素反应，并应用未被接管的反应加伤。因此修改伤害元素可以改变反应种类。后续扣血与反应处理沿用这次反应判定，不再根据扣血后的牌桌重新判断。

同组各次伤害分别广播本事件；此前伤害的扣血和附着已经生效，但本组的伤害后响应尚未调用。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::damage_calculation event{ .source = givm::character_id{}, .target = {}, .value = 3, .type = givm::damage_type::physical, .flags = {} };
    // 增加 1 点伤害，并把倍率调整为 2 倍。
    ++event.value;
    event.multiplier_numerator = 2;
    std::println("基础伤害: {}", event.value);
    std::println("伤害倍率: {}/{}", event.multiplier_numerator, event.multiplier_denominator);
}
```

输出

```text
基础伤害: 4
伤害倍率: 2/1
```

## 参阅

| | |
| --- | --- |
| [`deal_damage`](../commands/deal_damage.md) | 伤害结算命令 |
