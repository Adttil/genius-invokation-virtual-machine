[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **discard_hand_card_input**

# givm::discard_hand_card_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
using discard_hand_card_input = hand_card_discard_effect;
```

单张手牌舍弃命令的动态输入。目标必须是在命令执行时仍有效的手牌。 配合 [`discard_hand_card`](../commands/discard_hand_card.md) 和 [`handle_context::invoke`](../../executor/handle_context/invoke.md) 使用。

## 注意

本类型是 [`hand_card_discard_effect`](../events/hand_card_discard_effect.md) 的别名，成员及其限定与该事件相同。`discard_hand_card::input_type` 指向此类型，响应可以用别名或原事件类型构造输入；将它用于输入不会单独触发广播，结算与通知仍由命令决定。
