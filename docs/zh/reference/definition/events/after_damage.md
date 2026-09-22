[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **after_damage**

# givm::after_damage

定义于头文件 `<givm/definition.hpp>`

```cpp
struct after_damage;
```

伤害及其元素附着结算完成后的通知。响应者可以据此处理受到伤害或造成伤害后的效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const damage_source_id` | 本次伤害的来源；只读 |
| `target` | `const character_id` | 受到本次伤害的角色；只读 |
| `value` | `const std::uint32_t` | 用于扣除生命的伤害值；不以目标原有生命为上限；只读 |
| `type` | `const damage_type` | 伤害种类；只读 |
| `flags` | `const damage_flags` | 伤害附加属性；只读 |
| `reaction` | `const elemental_reaction` | 本次伤害在属性修饰结束后判定的反应，默认为 none；只读 |

## 时机

[`deal_damage`](../commands/deal_damage.md) 逐段完成伤害计算、扣血、击倒、元素附着及默认反应实体生成，整组结束后才按实际伤害顺序广播本事件。轮到某次伤害时，先完成它的元素反应后通知，再开始伤害后通知。击倒时已删除的 attachment 不参与本事件；若此前已判定终局，则不再广播本组尚未完成的通知。

响应函数在此时才被调用，读取的是当前牌桌；本组生成且仍在场的反应实体可以影响响应另行造成的伤害。事件成员保留对应伤害的结算结果，`reaction` 不根据此时的附着重新判断。前一个完成通知返回的程序可能继续修改牌桌，后面的通知会看到这些修改。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::after_damage event{ .source = givm::character_id{}, .target = {}, .value = 3, .type = givm::damage_type::physical, .flags = {} };
    std::println("结算伤害: {}", event.value);
    std::println("物理伤害: {}", event.type == givm::damage_type::physical);
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
