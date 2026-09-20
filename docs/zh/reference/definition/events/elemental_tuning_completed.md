[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_tuning_completed**

# givm::elemental_tuning_completed

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_tuning_completed;
```

元素调和完成后的通知，记录实际转换的结果。此时所选手牌已经离场，骰子转换已经完成。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |
| `from` | `const elemental_dice` | 转换或调和前的骰子种类；只读 |
| `to` | `const elemental_dice` | 转换或调和后的骰子种类；只读 |

## 注意

`card` 保留所用手牌的历史身份；在清理前仍可通过原 ID 读取它的定义和最后状态。`to` 是修饰响应结算后的最终元素，可能为万能元素。转换不另外产生骰子消耗或产生通知。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::elemental_tuning_completed event{ .card = {}, .from = givm::elemental_dice::hydro, .to = givm::elemental_dice::pyro };
    std::println("调和为火元素骰: {}", event.to == givm::elemental_dice::pyro);
}
```

输出

```text
调和为火元素骰: true
```
