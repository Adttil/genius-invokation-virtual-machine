[givm](../../reference.md) / [牌桌](../table.md) / **deck_card_status_id**

# givm::deck_card_status_id

定义于头文件 `<givm/table.hpp>`

```cpp
class deck_card_status_id;
```

牌库卡牌上的状态在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](deck_card_status_id/constructor.md) | 默认构造保持平凡，未初始化的 ID 须先赋值 |
| [`operator=`](deck_card_status_id/operator_assign.md) | 复制或移动同类 ID |
| [`value`](deck_card_status_id/value.md) | 取得不含类别标志的完整编码字，供读取或保存 |
| [`index`](deck_card_status_id/index.md) | 取得当前实体的索引 |
| [`player_id`](deck_card_status_id/player_id.md) | 取得所属玩家的 ID |
| [`deck_card_id`](deck_card_status_id/deck_card_id.md) | 取得该卡牌状态所属卡牌的 ID |
| [`operator==`](deck_card_status_id/operator_equal.md) | 比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌 |

卡牌状态还提供 `deck_card_id()`。

## 非成员函数

```cpp
friend constexpr bool operator==(deck_card_status_id, deck_card_status_id) = default;
```

比较编码的身份是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。
