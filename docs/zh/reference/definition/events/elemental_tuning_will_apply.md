[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_tuning_will_apply**

# givm::elemental_tuning_will_apply

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_tuning_will_apply;
```

元素调和生效前的事件。响应者可以取消这次调和。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `from` | `const elemental_dice` | 转换或调和前的骰子种类；只读 |
| `to` | `const elemental_dice` | 转换或调和后的骰子种类；只读 |
| `cancelled` | `bool` | 是否取消调和，初始为 false |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::elemental_tuning_will_apply event{ .card = {}, .from = givm::elemental_dice::hydro, .to = givm::elemental_dice::pyro };
    event.cancelled = true;
    std::println("取消调和: {}", event.cancelled);
}
```

输出

```text
取消调和: true
```
