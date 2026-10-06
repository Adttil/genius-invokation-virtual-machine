[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **after_damage**

# givm::after_damage

定义于头文件 `<givm/definition.hpp>`

一个角色在同一段中受到的伤害摘要，在结算点处理。多次命中合并为一个事件，响应读取届时的牌桌。

## 成员对象

所有成员只读。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 受伤角色 |
| `value` | `const std::uint32_t` | 本段最终伤害值之和，饱和至 `UINT32_MAX` |
| `type` | `const damage_type_mask` | 本段所有伤害类型的位或 |
| `flags` | `const damage_flags` | 本段所有伤害属性的位或 |
| `reaction` | `const elemental_reaction_mask` | 本段触发的反应槽位集合 |
| `defeated` | `const bool` | 段收尾时是否实际确认了死亡 |

事件不包含单击来源。以 `event.type[damage_type::pyro]` 或 `event.reaction[elemental_reaction::melt]` 判断集合成员。先处理混合和入手通知，再依首次受伤顺序处理非致命摘要，最后处理致命摘要；后者之前完成附属清理及离场通知。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::after_damage event{ .target = {}, .value = 3, .type = givm::damage_type::physical, .flags = {} };
    std::println("结算伤害: {}", event.value);
    std::println("物理伤害: {}", event.type[givm::damage_type::physical]);
}
```

输出

```text
结算伤害: 3
物理伤害: true
```

## 参阅

| | |
| --- | --- |
| [`deal_damage`](../commands/deal_damage.md) | 伤害结算命令 |
