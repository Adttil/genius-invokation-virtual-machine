[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **energy_changed**

# givm::energy_changed

定义于头文件 `<givm/definition.hpp>`

```cpp
struct energy_changed;
```

角色充能变化后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 充能已改变的角色；只读 |
| `previous` | `const std::uint32_t` | 变化前的充能；只读 |
| `current` | `const std::uint32_t` | 变化后的充能；只读 |

## 注意

行动支付充能时也发出本通知。支付先完成骰子与充能扣除，再处理非零骰子支付的 `dice_removed`，随后处理非零充能支付的本事件。费用已由报价确定，不在支付时通过 `changing_energy` 重新修改。

[`set_energy`](../commands/set_energy.md) 与 [`modify_energy`](../commands/modify_energy.md) 直接修改充能，不发送本通知。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::energy_changed event{ .target = {}, .previous = 1, .current = 3 };
    std::println("变化前: {}", event.previous);
    std::println("变化后: {}", event.current);
}
```

输出

```text
变化前: 1
变化后: 3
```
