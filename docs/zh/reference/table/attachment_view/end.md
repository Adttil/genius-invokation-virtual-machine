[givm](../../../reference.md) / [牌桌](../../table.md) / [attachment_view](../attachment_view.md) / **end**

# givm::attachment_view::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto end(this const auto& self);
```

取得这个角色附属实体的单实体范围终点。

## 参数

|  |  |
| --- | --- |
| `self` | 当前实体的只读视图 |

## 返回值

单实体范围的尾后迭代器。实体有效时范围包含一个元素，否则与 `begin()` 相等。
