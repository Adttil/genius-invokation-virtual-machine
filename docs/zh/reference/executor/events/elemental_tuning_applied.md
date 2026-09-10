[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **elemental_tuning_applied**

# givm::elemental_tuning_applied

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct elemental_tuning_applied;
```

元素调和完成后的通知。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `from` | `const elemental_dice` | 转换或调和前的骰子种类；只读 |
| `to` | `const elemental_dice` | 转换或调和后的骰子种类；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::elemental_tuning_applied event{ .card = {}, .from = givm::elemental_dice::hydro, .to = givm::elemental_dice::pyro };
    std::println("调和为火元素骰: {}", event.to == givm::elemental_dice::pyro);
}
```

输出

```text
调和为火元素骰: true
```
