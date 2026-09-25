[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **discard_hand_card**

# givm::discard_hand_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct discard_hand_card
{
    using input_type = discard_hand_card_input;

    relative_player player = relative_player::self;
    definition_id<card_definition> definition{};
    std::uint32_t count = 1;
};
```

同时舍弃一组手牌，再逐张执行自身的舍弃效果和全场通知。元素调和、打出牌和手牌溢出的移除不属于舍弃。

## 成员类型

| | |
| --- | --- |
| `input_type` | [`discard_hand_card_input`](../command_inputs/discard_hand_card_input.md)，动态模式下的输入类型 |

## 参数形式

指定 `definition` 时，按 `player` 的有效手牌遍历顺序，选择至多 `count` 张采用该定义的牌。`count` 默认为 1；设为 `std::numeric_limits<std::uint32_t>::max()` 可选择全部匹配牌。没有匹配牌或 `count == 0` 时无效果。玩家相对于命令执行时的本方确定，见 [relative_player](relative_player.md)。

默认构造 `discard_hand_card{}` 时，消费响应通过 `invoke` 提交的一个 [`discard_hand_card_input`](../command_inputs/discard_hand_card_input.md)，其中 `cards` 是按结算顺序排列的有效手牌列表；目标不得重复，允许为空。动态模式的数量由数组长度决定，不读取命令的 `count`。

## 结算

1. 确定本次全部目标，将全部目标手牌及其附属状态标记为离场。
2. 对第一张牌，仅向自身发送 `hand_card_discard_effect`，完整执行返回的程序。
3. 为该牌全场广播 [`hand_card_discarded`](../events/hand_card_discarded.md)，完整执行所有响应。
4. 对下一张牌重复步骤 2、3，直到本批全部结算完。

自身效果和全场响应看到的所有本批目标都已离场。期间生成的新牌不加入本批；需要每张舍弃后重新选择目标时，使用多条独立命令。

离场后的卡牌仍可通过 ID 读取其定义和状态，直到安全清理。自身舍弃效果与全场通知是两种事件：定义仅响应后者不代表它具有自身舍弃效果。

## 参阅

- [`discard_deck_cards`](discard_deck_cards.md)：批量舍弃牌堆顶的牌。
