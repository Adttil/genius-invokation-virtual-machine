[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **elemental_tuning_modification**

# givm::elemental_tuning_modification

定义于头文件 `<givm/definition.hpp>`

```cpp
struct elemental_tuning_modification;
```

元素调和转换骰子前的修饰事件。此时使用的手牌及其附属状态仍然有效，响应可以改变本次转换的目标元素，例如将结果改为万能骰。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 用于调和的手牌 ID，只读 |
| `from` | `const elemental_dice` | 选中的原骰子元素，只读 |
| `to` | `elemental_dice` | 转换后的元素，初始为出战角色自身元素，可修改 |

## 注意

响应可以返回后续效果入口，但在调和本体执行前，不得转移或移除所选手牌，且必须为当前玩家保留至少一枚 `from` 骰子。定义源负责遵守此前提，执行时不检查；违反约定属于未定义行为。

本事件不放宽选骰条件：即使响应会将 `to` 改为万能元素，输入仍不能选万能骰或与出战角色同元素的骰子。

修饰响应结算完成后，执行器转换一枚骰子并移除手牌，再广播 [`elemental_tuning_completed`](elemental_tuning_completed.md)。本次转换不广播骰子产生或消耗事件。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::elemental_tuning_modification event{
        .card = {}, .from = givm::elemental_dice::hydro, .to = givm::elemental_dice::pyro
    };
    event.to = givm::elemental_dice::omni;
    std::println("调和为万能骰: {}", event.to == givm::elemental_dice::omni);
}
```

输出

```text
调和为万能骰: true
```
