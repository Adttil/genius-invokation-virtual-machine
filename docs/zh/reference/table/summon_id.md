[givm](../../reference.md) / [牌桌](../table.md) / **summon_id**

# givm::summon_id

定义于头文件 `<givm/table.hpp>`

```cpp
struct summon_id;
```

召唤物在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player_id` | [`player_id`](player_id.md) | 所属玩家的 ID |
| `index` | `size_t` | 该实体的标识值；不表示筛除已移除实体后的排列位置 |

## 非成员函数

```cpp
friend constexpr bool operator==(summon_id, summon_id) = default;
```

比较各成员是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。
