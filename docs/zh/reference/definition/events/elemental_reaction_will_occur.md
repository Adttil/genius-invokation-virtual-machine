[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_reaction_will_occur**

# givm::elemental_reaction_will_occur

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_reaction_will_occur;
```

元素反应判定后的效果选择事件。响应者可以为已确定的反应指定替代标签，供后续的伤害计算和完成响应执行替代效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const element_application_source_id` | 引发这次反应的元素附着来源；只读 |
| `target` | `const character_id` | 发生元素反应的角色；只读 |
| `incoming_element` | `const element` | 本次附着的元素；只读 |
| `reacted_aura` | `const element_aura` | 发生反应前的元素附着；只读 |
| `reaction` | `const elemental_reaction` | 本次元素反应的种类；只读 |
| `cause` | `const element_application_cause` | 由伤害还是独立效果引发，初始为 effect；只读 |
| `replacement_reaction` | `tag_id` | 替代反应的标签，可修改；初始为空，表示使用默认反应效果 |

## 注意

由伤害引发时，本事件属于整组准备阶段。初始描述经过 [`damage_preparation`](damage_preparation.md) 后展开具体命中；默认反应派生伤害则直接判定反应。两者在判定发生反应时均广播本事件，先于整组的 [`damage_calculation`](damage_calculation.md)。`reaction`、`reacted_aura` 和 `incoming_element` 均已固定；之后即使响应效果改变附着，也不重新判定本次反应。独立 [`apply_element`](../commands/apply_element.md) 也在反应判定后广播本事件。没有发生反应时不广播。

响应者按[全场广播顺序](../events.md#全场广播)读取和修改 `replacement_reaction`。后一次写入无条件覆盖前一次，写回空标签会恢复默认效果；没有额外的标签优先级。本事件结束后，后续事件中的标签只读。

最终标签非空时，取消本次反应的默认数值加成、派生伤害、实体生成和超载切人。默认附着消耗始终按原始反应处理；`reaction` 仍保留原值，后续按反应种类判断的效果和 [`after_elemental_reaction`](after_elemental_reaction.md) 通知仍认可该反应发生。

标签本身不执行效果。响应者在已有的数值、反应后或伤害后事件中读取它并完成替代效果；其中另行调用的 `deal_damage` 独立结算。仅返回响应程序而不修改标签，不会取消默认反应效果。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "替代甲", "替代乙" };
    givm::elemental_reaction_will_occur event{ .source = givm::character_id{}, .target = {}, .incoming_element = givm::element::pyro, .reacted_aura = givm::element_aura::hydro, .reaction = givm::elemental_reaction::vaporize };
    event.replacement_reaction = ids.get_tag_id("替代甲");
    event.replacement_reaction = ids.get_tag_id("替代乙");
    std::println("最终替代标签: {}", ids.tag_name(event.replacement_reaction));
    std::println("仍认可蒸发: {}", event.reaction == givm::elemental_reaction::vaporize);
}
```

输出

```text
最终替代标签: 替代乙
仍认可蒸发: true
```

## 参阅

| | |
| --- | --- |
| [`apply_element`](../commands/apply_element.md) | 元素附着命令 |
