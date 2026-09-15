[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_effect**

# givm::damage_effect

定义于头文件 `<givm/definition.hpp>`

```cpp
struct damage_effect;
```

扣除生命前的伤害结算事件。护盾和其他减伤效果可以在这里减少最终伤害。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const damage_source_id` | 本次伤害的来源；只读 |
| `target` | `const character_id` | 将扣除生命的角色；只读 |
| `value` | `std::uint32_t` | 将用于扣除生命的伤害值，可由响应者减少 |
| `type` | `const damage_type` | 伤害种类；只读 |
| `flags` | `const damage_flags` | 伤害附加属性；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::damage_effect event{ .source = givm::character_id{}, .target = {}, .value = 3, .type = givm::damage_type::physical, .flags = {} };
    // 抵挡 2 点伤害。
    event.value -= 2;
    std::println("剩余伤害: {}", event.value);
}
```

输出

```text
剩余伤害: 1
```

## 参阅

| | |
| --- | --- |
| [`absorb_damage_by_count`](../commands/absorb_damage_by_count.md) | 消耗实体计数以抵挡伤害的命令 |
| [`deal_damage`](../commands/deal_damage.md) | 伤害结算命令 |
