[givm](../../reference.md) / [枚举值](../enums.md) / **element_application_cause**

# givm::element_application_cause

定义于头文件 `<givm/enums/element_application_cause.hpp>`

```cpp
enum class element_application_cause : std::uint8_t
{
    effect,
    damage
};
```

本次施加元素的来源，区分独立效果施加元素与伤害附带的元素。

## 枚举值

|  |  |
| --- | --- |
| `effect` | 独立效果 |
| `damage` | 伤害 |

## 示例

```cpp
#include <print>

#include <givm/executor.hpp>

int main()
{
    const givm::after_elemental_reaction event{
        .source = givm::character_id{ { 0 }, 0 },
        .target = { { 1 }, 0 },
        .incoming_element = givm::element::pyro,
        .reacted_aura = givm::element_aura::cryo,
        .reaction = givm::elemental_reaction::melt,
        .cause = givm::element_application_cause::damage
    };
    std::println("本次反应的元素来源: {}",
        event.cause == givm::element_application_cause::damage ? "伤害" : "独立效果");
}
```

输出

```text
本次反应的元素来源: 伤害
```
