[givm](../../reference.md) / [牌桌](../table.md) / **optional_tag_id**

# givm::optional_tag_id

定义于头文件 `<givm/table.hpp>`

```cpp
class optional_tag_id;
```

允许没有标签的身份，适用于查找结果或尚未指定的能量分类。默认构造和 `nullptr` 表示空值；已取得的 `tag_id` 可隐式转换为本类型。

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [(构造函数)](optional_tag_id/constructor.md) | 构造空值或保存标签 |
| [`operator bool`](optional_tag_id/operator_bool.md) | 判断是否持有标签 |
| [`has_value`](optional_tag_id/operator_bool.md) | 同上 |
| [`get`](optional_tag_id/get.md) | 取得可用于索引的 `tag_id` |
| [`operator*`](optional_tag_id/get.md) | 同上 |
| [`value`](optional_tag_id/value.md) | 取得标签索引或空值编码 |
| [`operator==`](optional_tag_id/operator_equal.md) | 比较可空身份 |

保存一个 `std::size_t`，空值使用其最大值。`get()` 只在 Debug 检查空值并抛出 `std::invalid_argument`；Release 须由调用方满足前提。
