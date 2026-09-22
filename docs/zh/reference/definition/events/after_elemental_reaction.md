[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **after_elemental_reaction**

# givm::after_elemental_reaction

定义于头文件 `<givm/definition.hpp>`

```cpp
struct after_elemental_reaction;
```

元素反应处理完成后的通知。它记录引发反应的元素、原有附着和反应来源，也携带本次反应选定的替代标签。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const element_application_source_id` | 引发这次反应的元素附着来源；只读 |
| `target` | `const character_id` | 发生元素反应的角色；只读 |
| `incoming_element` | `const element` | 本次附着的元素；只读 |
| `reacted_aura` | `const element_aura` | 发生反应前的元素附着；只读 |
| `reaction` | `const elemental_reaction` | 本次元素反应的种类；只读 |
| `cause` | `const element_application_cause` | 由伤害还是独立效果引发，初始为 effect；只读 |
| `replacement_reaction` | `const tag_id` | 本次反应采用的替代标签，空值表示默认反应效果；只读 |

## 时机

由伤害触发时，各段伤害先分别完成扣血、击倒、元素附着及默认反应实体生成；须等待整组结算结束，并完成已登记的超载切人及其通知，才在引发该反应的伤害后通知之前广播本事件。反应派生伤害也在这些完成通知之前结算；反应信息仍记录引发反应时的元素与附着。若此前已判定终局，则不再进行剩余完成通知。

独立 [`apply_element`](../commands/apply_element.md) 在相应反应处理中完成默认实体生成；其派生伤害也逐段完成扣血与附着，全部结算并完成已登记的超载切人后再进行完成通知。没有发生反应时不广播本事件。

非空 `replacement_reaction` 不取消本通知，`reaction` 仍是原始元素反应。响应者可在此根据标签完成替代效果，例如造成额外伤害；其中的 `deal_damage` 独立结算，不追加进刚完成的伤害组。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::after_elemental_reaction event{ .source = givm::character_id{}, .target = {}, .incoming_element = givm::element::pyro, .reacted_aura = givm::element_aura::hydro, .reaction = givm::elemental_reaction::vaporize };
    std::println("发生蒸发: {}", event.reaction == givm::elemental_reaction::vaporize);
    std::println("由独立效果附着: {}", event.cause == givm::element_application_cause::effect);
}
```

输出

```text
发生蒸发: true
由独立效果附着: true
```

## 参阅

| | |
| --- | --- |
| [`apply_element`](../commands/apply_element.md) | 元素附着命令 |
