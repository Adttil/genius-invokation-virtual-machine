[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **damage_effect**

# givm::damage_effect

加值和倍率已经处理后的即时减伤、护盾时机。只有 `std::uint32_t value` 可修改；`source`、`target`、`type`、`flags` 和 `reaction_id reaction` 只读。响应结束后以最终值扣血，并登记当前段的伤害摘要。反应槽位不会根据期间改变的附着重新判定。

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
| [`modify_combat_status_state`](../commands/modify_combat_status_state.md) | 按增量修改出战状态的层数和本回合次数 |
| [`deal_damage`](../commands/deal_damage.md) | 伤害结算命令 |
