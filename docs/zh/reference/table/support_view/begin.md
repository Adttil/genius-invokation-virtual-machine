[givm](../../../reference.md) / [牌桌](../../table.md) / [support_view](../support_view.md) / **begin**

# givm::support_view::begin

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto begin(this const auto& self);
```

取得这个支援的单实体范围起点。

## 参数

|  |  |
| --- | --- |
| `self` | 当前实体的只读视图 |

## 返回值

单实体范围的起始迭代器。实体有效时，解引用取得该实体的只读 [`support_view`](../support_view.md)；实体无效时与 `end()` 相等。

## 注意

范围依赖访问对象本身的存活。遍历有效实体时执行一次循环；遍历已移除实体时不执行循环。
