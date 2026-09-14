[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **supports**

# givm::player_view::supports

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto supports() const;
```

遍历该玩家的支援。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`support_view`](../support_view.md) 的范围。

## 注意

遍历所得访问对象的存活约定见[实体的身份与访问](../entity_access.md)。
