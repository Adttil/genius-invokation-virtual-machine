[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **skills**

# givm::character_view::skills

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto skills() const;
```

遍历该角色的技能。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`skill_view`](../skill_view.md) 的范围。

## 注意

角色必须有效。范围中的实体均为只读视图。
