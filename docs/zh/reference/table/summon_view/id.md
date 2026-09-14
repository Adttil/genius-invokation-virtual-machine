[givm](../../../reference.md) / [牌桌](../../table.md) / [summon_view](../summon_view.md) / **id**

# givm::summon_view::id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr summon_id id() const;
```

取得召唤物的身份，以便之后从牌桌再次访问它。

## 返回值

该实体的 [`summon_id`](../summon_id.md)。

## 注意

实体必须有效。ID 的保存期限见[实体的身份与访问](../entity_access.md)。
