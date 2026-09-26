[givm](../../../reference.md) / [定义](../../definition.md) / [命令输入](../command_inputs.md) / **draw_cards_input**

# givm::draw_cards_input

定义于头文件 `<givm/definition/commands.hpp>`

```cpp
struct draw_cards_input
{
    std::span<const deck_card_id> cards;
};
```

从牌堆抽取一组指定卡牌的输入，配合 [`draw_cards`](../commands/draw_cards.md) 使用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cards` | `std::span<const deck_card_id>` | 按抽取顺序排列的牌堆卡牌列表 |

## 注意

目标须在命令开始执行时有效，不得重复，列表允许为空。每张牌进入其所属玩家的手牌，允许同一批指定双方的牌。卡牌原有的状态和附属状态随牌转移；未抽取牌的相对顺序不变。

本次全部抽取完成后，按输入顺序逐张结算 [`card_drawn`](../events/card_drawn.md)。手牌已满时仍移走相应牌堆卡牌及其附属状态，但它不进入手牌，也不触发抽牌或舍弃通知。

[`invoke`](../../executor/handle_context/invoke.md) 复制数组内容，返回后不再借用原数组。数组须在复制期间保持有效。

## 参阅

| | |
| --- | --- |
| [`draw_cards`](../commands/draw_cards.md) | 抽牌命令 |
| [`deck_card_id`](../../table/deck_card_id.md) | 牌堆卡牌的实体标识 |
