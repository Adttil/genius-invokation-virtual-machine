[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_reaction_will_occur**

# givm::elemental_reaction_will_occur

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_reaction_will_occur;
```

元素反应生效前的事件。响应者可以识别反应种类，并接管这次反应的附着变化与派生效果。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const element_application_source_id` | 引发这次反应的元素附着来源；只读 |
| `target` | `const character_id` | 发生元素反应的角色；只读 |
| `incoming_element` | `const element` | 本次附着的元素；只读 |
| `reacted_aura` | `const element_aura` | 发生反应前的元素附着；只读 |
| `reaction` | `const elemental_reaction` | 本次元素反应的种类；只读 |
| `cause` | `const element_application_cause` | 由伤害还是独立效果引发，初始为 effect；只读 |
| `already_handled` | `bool` | 是否由响应者接管反应；设为 `true` 时跳过默认附着变化、默认派生伤害及默认反应实体生成 |

## 注意

伤害中的反应加成与本事件分别处理，由 [`damage_calculation`](damage_calculation.md) 的 `already_handled_reaction` 控制。修改本事件的 `already_handled` 不会撤销此前计算的伤害加成。

由伤害引发时，本事件在该次扣血及击倒处理后广播；若已判定终局则不广播。反应种类及 `reacted_aura` 已在 [`damage_preparation`](damage_preparation.md) 结束后、数值计算前确定，之后的附着变化不会重新决定本次反应。响应接管后须自行完成替代处理；仅返回程序而不设置 `already_handled` 不会跳过默认效果。

未接管时，本事件结束后处理默认附着变化、派生伤害或反应实体生成。实体生成及其重复生成响应完成后，才继续下一段伤害；反应后和伤害后的通知仍等整组结算结束再广播。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::elemental_reaction_will_occur event{ .source = givm::character_id{}, .target = {}, .incoming_element = givm::element::pyro, .reacted_aura = givm::element_aura::hydro, .reaction = givm::elemental_reaction::vaporize };
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
