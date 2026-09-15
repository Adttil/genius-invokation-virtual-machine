[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_reaction_will_occur**

# givm::elemental_reaction_will_occur

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_reaction_will_occur;
```

元素反应生效前的事件。响应者可以识别反应种类，并接管这次反应引起的附着变化。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const element_application_source_id` | 引发这次反应的元素附着来源；只读 |
| `target` | `const character_id` | 发生元素反应的角色；只读 |
| `incoming_element` | `const element` | 本次附着的元素；只读 |
| `reacted_aura` | `const element_aura` | 发生反应前的元素附着；只读 |
| `reaction` | `const elemental_reaction` | 本次元素反应的种类；只读 |
| `cause` | `const element_application_cause` | 由伤害还是独立效果引发，初始为 effect；只读 |
| `already_handled` | `bool` | 是否由响应者处理附着变化；设为 `true` 时跳过默认的附着消耗 |

## 注意

伤害中的反应加成与本事件分别处理，由 [`damage_calculation`](damage_calculation.md) 的 `already_handled_reaction` 控制。修改本事件的 `already_handled` 不会撤销此前计算的伤害加成。

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
