[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **discard_hand_card_input**

# givm::discard_hand_card_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct discard_hand_card_input
{
    std::span<const hand_card_id> cards;
};
```

同时舍弃多张手牌的输入。 配合 [`discard_hand_card`](../commands/discard_hand_card.md) 使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cards` | `std::span<const hand_card_id>` | 本次舍弃的手牌列表 |

## 注意

`cards` 按结算顺序指定有效手牌，允许为空，目标不得重复。先将全部目标及其附属状态标记离场，再按输入顺序逐张结算自身舍弃效果和全场舍弃通知。

[`invoke`](../../executor/handle_context/invoke.md) 复制数组内容，返回后不再借用原数组。数组须在复制期间保持有效。目标须在命令开始执行时有效。
