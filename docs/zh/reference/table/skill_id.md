[givm](../../reference.md) / [牌桌](../table.md) / **skill_id**

# givm::skill_id

定义于头文件 `<givm/table.hpp>`

```cpp
class skill_id;
```

技能在一张牌桌中的身份。使用此 ID 可以通过 [`table::operator[]`](table/operator_subscript.md) 再次取得相应实体。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](skill_id/constructor.md) | 默认构造保持平凡，未初始化的 ID 须先赋值 |
| [`operator=`](skill_id/operator_assign.md) | 复制或移动同类 ID |
| [`value`](skill_id/value.md) | 取得不含类别标志的完整编码字，供读取或保存 |
| [`index`](skill_id/index.md) | 取得当前实体的索引 |
| [`player_id`](skill_id/player_id.md) | 取得所属玩家的 ID |
| [`character_id`](skill_id/character_id.md) | 取得该技能或附属实体所属角色的 ID |
| [`operator==`](skill_id/operator_equal.md) | 比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌 |

技能和角色附属实体还提供 `character_id()`。

## 非成员函数

```cpp
friend constexpr bool operator==(skill_id, skill_id) = default;
```

比较编码的身份是否相等；比较不检查实体是否尚未移除。

## 注意

ID 本身不包含牌桌身份。清理实体后，原有 ID 可能失效；详见[实体的身份与访问](entity_access.md)。
