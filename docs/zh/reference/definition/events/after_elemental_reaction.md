[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **after_elemental_reaction**

# givm::after_elemental_reaction

定义于头文件 `<givm/definition.hpp>`

```cpp
struct after_elemental_reaction;
```

元素反应处理完成后的通知。它记录引发反应的元素、原有附着和反应来源。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const element_application_source_id` | 引发这次反应的元素附着来源；只读 |
| `target` | `const character_id` | 发生元素反应的角色；只读 |
| `incoming_element` | `const element` | 本次附着的元素；只读 |
| `reacted_aura` | `const element_aura` | 发生反应前的元素附着；只读 |
| `reaction` | `const elemental_reaction` | 本次元素反应的种类；只读 |
| `cause` | `const element_application_cause` | 由伤害还是独立效果引发，初始为 effect；只读 |

## 时机

由伤害触发时，须等待所属伤害组内全部伤害完成扣血、击倒处理与元素附着，随后在引发该反应的伤害后通知之前广播。反应派生伤害也在这些完成通知之前结算；反应信息仍记录引发反应时的元素与附着。若此前已判定终局，则不再进行剩余完成通知。

独立 [`apply_element`](../commands/apply_element.md) 引发的反应也先完成其派生伤害的扣血与附着，再进行完成通知。没有发生反应时不广播本事件。

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
