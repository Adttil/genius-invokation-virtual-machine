[givm](../../../reference.md) / [牌桌](../../table.md) / [support_view](../support_view.md) / **id**

# givm::support_view::id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr support_id id() const;
```

取得支援的身份，以便之后从牌桌再次访问它。

## 返回值

该实体的 [`support_id`](../support_id.md)。

## 注意

实体必须有效。ID 的保存期限见[实体的身份与访问](../entity_access.md)。
