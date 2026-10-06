[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **discard_hand_card**

# givm::discard_hand_card

定义于头文件 `<givm/definition.hpp>`

```cpp
struct discard_hand_card_error;

struct discard_hand_card
{
    using error_type = discard_hand_card_error;

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
| [`error_type`](#编译检查) | `discard_hand_card_error` 的别名，即本命令的编译检查错误类型 |

## 参数形式

指定 `definition` 时，按 `player` 的有效手牌遍历顺序，选择至多 `count` 张采用该定义的牌。`count` 默认为 1；设为 `std::numeric_limits<std::uint32_t>::max()` 可选择全部匹配牌。没有匹配牌或 `count == 0` 时无效果。玩家相对于命令执行时的本方确定，见 [relative_player](relative_player.md)。

默认构造 `discard_hand_card{}` 时，消费响应通过 `invoke` 提交的一个 [`discard_hand_card_input`](../command_inputs/discard_hand_card_input.md)，其中 `cards` 是按结算顺序排列的有效手牌列表；目标不得重复，允许为空。动态模式的数量由数组长度决定，不读取命令的 `count`。

## 结算

1. 确定本次全部目标，将全部目标手牌及其附属状态标记为离场。
2. 对第一张牌，仅向自身发送 `this_hand_card_discard`，完整执行返回的程序。
3. 为该牌全场广播 [`hand_card_discarded`](../events/hand_card_discarded.md)，完整执行所有响应。
4. 对下一张牌重复步骤 2、3，直到本批全部结算完。

自身效果和全场响应看到的所有本批目标都已离场。期间生成的新牌不加入本批；需要每张舍弃后重新选择目标时，使用多条独立命令。

离场后的卡牌仍可通过 ID 读取其定义和状态，直到安全清理。自身舍弃效果与全场通知是两种事件：定义仅响应后者不代表它具有自身舍弃效果。

## 编译检查

```cpp
struct discard_hand_card_error;
```

`discard_hand_card::error_type` 是 `givm::discard_hand_card_error` 的别名。`discard_hand_card_error` 是本命令的结构化编译错误，`discard_hand_card_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `card_definition` 定义数量 |

### `discard_hand_card_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 参阅

- [`discard_deck_cards`](discard_deck_cards.md)：批量舍弃牌堆顶的牌。
