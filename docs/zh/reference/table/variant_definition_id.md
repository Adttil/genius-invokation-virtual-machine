[givm](../../reference.md) / [牌桌](../table.md) / **variant_definition_id**

# givm::variant_definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
template<definition_category... Categories>
class variant_definition_id;
```

可以指向多种定义类别的身份。类别和相应索引共同存放在一个 `std::uint64_t` 中；在取得具体类别后，提取单类别 ID 进行访问。

## 模板参数

`Categories` 列出允许保存的类别，至少一个；包含 `definition_category::null` 时允许空值。例如 `variant_definition_id<definition_category::null, definition_category::card>` 表示可空的卡牌定义 ID。

## 成员

| 名称 | 说明 |
| --- | --- |
| [`(构造函数)`](variant_definition_id/constructor.md) | 从允许的 ID、另一种允许类别的 ID 集合或编码值构造 |
| [`operator=`](variant_definition_id/operator_assign.md) | 赋值 |
| [`category`](variant_definition_id/category.md) | 取得当前类别 |
| [`holds`](variant_definition_id/holds.md) | 判断当前类别 |
| [`get`](variant_definition_id/get.md) | 提取指定类别的非空 ID |
| [`get_if`](variant_definition_id/get_if.md) | 类别相符时返回相应的可空 ID |
| [`visit`](variant_definition_id/visit.md) | 按当前类别调用访问器 |
| [`value`](variant_definition_id/value.md) | 取得整个编码字 |
| [`operator bool`](variant_definition_id/operator_bool.md) | 可空类型判断是否非空 |
| [`has_value`](variant_definition_id/operator_bool.md) | 同上 |
| [`operator*`](variant_definition_id/get.md) | 可空且只有一种实际类别时提取非空 ID |
| [`operator==`](variant_definition_id/operator_equal.md) | 比较同一种 ID 集合中的值 |

## 注意

可空类型默认构造为空，也接受 `nullptr`。非空类型的默认构造保持平凡，未初始化对象必须先赋值。可空和非空类型不能相互当作同一种类型使用；转换只允许把类别集合扩大。

`get_if` 返回拥有值的可空 ID，不返回指向内部存储的指针。`visit` 向访问器传入按值构造的单类别 ID；空值传入 `nullptr`，不能保存指向访问器收到的临时 ID 的引用。

Debug 检查整数编码、空值编码及 `get` 的类别前提，失败抛出 `std::invalid_argument`。Release 不执行这些检查。
