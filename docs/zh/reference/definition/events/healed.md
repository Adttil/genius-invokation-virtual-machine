[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **healed**

# givm::healed

定义于头文件 `<givm/definition.hpp>`

```cpp
struct healed;
```

角色治疗完成后的通知。广播时牌桌已更新生命值；`value` 是经生命上限截断后的实际恢复量。满血等情况仍产生通知，此时值为 `0`。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const effect_source_id` | 本次治疗的来源；只读 |
| `target` | `const character_id` | 已接受治疗的角色；只读 |
| `value` | `const std::uint32_t` | 本次实际恢复的生命值；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::healed event{ .source = givm::support_id{}, .target = {}, .value = 2 };
    std::println("治疗量: {}", event.value);
}
```

输出

```text
治疗量: 2
```
