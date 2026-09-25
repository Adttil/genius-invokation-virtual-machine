[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **discard_deck_cards_input**

# givm::discard_deck_cards_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct discard_deck_cards_input
{
    player_id player;
    std::uint32_t count;
};
```

默认构造的 [`discard_deck_cards{}`](../commands/discard_deck_cards.md) 所需的动态输入。响应通过 `invoke` 提交目标玩家与舍弃数量；执行时舍弃该玩家牌堆顶至多 `count` 张牌。

此类型只描述输入，不广播。卡牌自身效果和全场通知分别使用 [`deck_card_discard_effect`](../events/deck_card_discard_effect.md) 与 [`deck_card_discarded`](../events/deck_card_discarded.md)。
