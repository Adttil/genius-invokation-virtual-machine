[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **healing**

# givm::healing

定义于头文件 `<givm/definition.hpp>`

```cpp
struct healing;
```

角色恢复生命前的治疗调整事件。响应者可以调整本次治疗量。满血或请求值为 `0` 时仍广播；所有响应程序完成后才按照目标当时的生命上限截断实际恢复值。范围治疗会为每个目标分别广播本事件并加血，全部目标完成后才依次广播 [`healed`](healed.md)。

动态 [`heal`](../commands/heal.md) 接受 [`heal_input`](../command_inputs/heal_input.md) 输入。增加生命上限的 [`increase_max_health_input`](../command_inputs/increase_max_health_input.md) 则是本事件的别名；该命令不会广播治疗量修饰事件。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const effect_source_id` | 本次治疗的来源；只读 |
| `target` | `const character_id` | 将接受治疗的角色；只读 |
| `value` | `std::uint32_t` | 将恢复的生命值，可由响应者调整 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::healing event{ .source = givm::support_id{}, .target = {}, .value = 2 };
    ++event.value;
    std::println("治疗量: {}", event.value);
}
```

输出

```text
治疗量: 3
```
