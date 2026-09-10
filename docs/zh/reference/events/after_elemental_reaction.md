[givm](../../reference.md) / [事件](../events.md) / **after_elemental_reaction**

# givm::after_elemental_reaction

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct after_elemental_reaction;
```

一次元素反应的处理已经结束。它记录引发反应的元素、原有附着和反应来源。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const element_application_source_id` | 引发这次反应的元素附着来源；只读 |
| `target` | `const character_id` | 发生元素反应的角色；只读 |
| `incoming_element` | `const element` | 本次附着的元素；只读 |
| `reacted_aura` | `const element_aura` | 发生反应前的元素附着；只读 |
| `reaction` | `const elemental_reaction` | 本次元素反应的种类；只读 |
| `cause` | `const element_application_cause` | 由伤害还是独立效果引发，初始为 effect；只读 |

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
| [`apply_element`](../instructions/apply_element.md) | 施加元素附着 |
