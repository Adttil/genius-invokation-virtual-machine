[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **attachments**

# givm::character_view::attachments

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto attachments() const;
```

遍历该角色的角色附属实体。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`attachment_view`](../attachment_view.md) 的范围。

## 注意

角色必须有效。范围中的实体均为只读视图。
