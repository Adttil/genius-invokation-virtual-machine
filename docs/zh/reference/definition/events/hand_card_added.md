[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **hand_card_added**

# givm::hand_card_added

定义于头文件 `<givm/definition.hpp>`

```cpp
struct hand_card_added
{
    const hand_card_id card;
};
```

通过非抽牌方式加入手牌后的全场通知。响应者可以通过牌的标识读取已经加入手牌的牌。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card` | `const hand_card_id` | 本次事件对应的牌标识；只读 |

## 注意

[`create_hand_card`](../commands/create_hand_card.md) 成功生成手牌后发出本通知。手牌已满而未生成新牌时，不发送通知。

抽牌仅发送 [`card_drawn`](card_drawn.md)，不会再发送本通知。需要响应任意方式加入手牌的定义，应同时响应这两种事件；这两类入手通知互斥，响应者按该次广播的顺序结算。

## 参阅

| | |
| --- | --- |
| [`create_hand_card`](../commands/create_hand_card.md) | 向手牌中生成一张指定牌的命令 |
| [`card_drawn`](card_drawn.md) | 抽牌成功后的通知 |
