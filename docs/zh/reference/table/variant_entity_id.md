[givm](../../reference.md) / [牌桌](../table.md) / **variant_entity_id**

# givm::variant_entity_id

定义于头文件 `<givm/table.hpp>`

```cpp
template<entity_category... Categories>
class variant_entity_id;
```

可以指向多种实体类别的身份。类别和相应索引共同存放在一个 `std::uint64_t` 中；在取得具体类别后，提取单类别 ID 进行访问。

## 模板参数

`Categories` 列出允许保存的类别，至少一个；包含 `entity_category::null` 时允许空值。例如 `variant_entity_id<entity_category::null, entity_category::hand_card>` 表示可空的卡牌实体 ID。

## 成员

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](variant_entity_id/constructor.md) | 从允许的 ID 或另一种允许类别的 ID 集合构造 |
| [`operator=`](variant_entity_id/operator_assign.md) | 赋值 |
| [`category`](variant_entity_id/category.md) | 取得当前类别 |
| [`holds`](variant_entity_id/holds.md) | 判断当前类别 |
| [`get`](variant_entity_id/get.md) | 提取指定类别的非空 ID |
| [`get_if`](variant_entity_id/get_if.md) | 类别相符时返回相应的可空 ID |
| [`player_id`](variant_entity_id/player_id.md) | 非空类型直接取得所属玩家的 ID |
| [`visit`](variant_entity_id/visit.md) | 按当前类别调用访问器 |
| [`value`](variant_entity_id/value.md) | 取得整个编码字 |
| [`operator bool`](variant_entity_id/operator_bool.md) | 可空类型判断是否非空 |
| [`has_value`](variant_entity_id/operator_bool.md) | 同上 |
| [`operator*`](variant_entity_id/get.md) | 可空且只有一种实际类别时提取非空 ID |
| [`operator==`](variant_entity_id/operator_equal.md) | 比较同一种 ID 集合中的值 |

## 注意

可空类型默认构造为空，也接受 `nullptr`。非空类型的默认构造保持平凡，未初始化对象必须先赋值。可空和非空类型不能相互当作同一种类型使用；转换只允许把类别集合扩大。

`get_if` 返回拥有值的可空 ID，不返回指向内部存储的指针。`visit` 向访问器传入按值构造的单类别 ID；空值传入 `nullptr`，不能保存指向访问器收到的临时 ID 的引用。

Debug 检查 `get` 的类别前提，失败抛出 `std::invalid_argument`。Release 不执行这一检查。
