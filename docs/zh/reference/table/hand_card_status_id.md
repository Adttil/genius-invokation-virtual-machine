[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_status_id**

# givm::hand_card_status_id

定义于头文件 `<givm/table.hpp>`

```cpp
class hand_card_status_id;
```

手牌上的状态在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](hand_card_status_id/constructor.md) | 默认构造保持平凡，未初始化的 ID 须先赋值 |
| [`operator=`](hand_card_status_id/operator_assign.md) | 复制或移动同类 ID |
| [`value`](hand_card_status_id/value.md) | 取得不含类别标志的完整编码字，供读取或保存 |
| [`index`](hand_card_status_id/index.md) | 取得当前实体的索引 |
| [`player_id`](hand_card_status_id/player_id.md) | 取得所属玩家的 ID |
| [`hand_card_id`](hand_card_status_id/hand_card_id.md) | 取得该卡牌状态所属卡牌的 ID |
| [`operator==`](hand_card_status_id/operator_equal.md) | 比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌 |

卡牌状态还提供 `hand_card_id()`。

## 非成员函数

```cpp
friend constexpr bool operator==(hand_card_status_id, hand_card_status_id) = default;
```

比较编码的身份是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。
