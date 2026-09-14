[givm](../../reference.md) / [牌桌](../table.md) / **hand_card_status_id**

# givm::hand_card_status_id

定义于头文件 `<givm/table.hpp>`

```cpp
struct hand_card_status_id;
```

手牌上的状态在一张牌桌中的身份。使用此 ID 可以通过 [`card_table::operator[]`](card_table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `card_id` | [`hand_card_id`](hand_card_id.md) | 所属卡牌的 ID |
| `index` | `size_t` | 该实体的标识值；不表示筛除已移除实体后的排列位置 |

## 非成员函数

```cpp
friend constexpr bool operator==(hand_card_status_id, hand_card_status_id) = default;
```

比较各成员是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。
