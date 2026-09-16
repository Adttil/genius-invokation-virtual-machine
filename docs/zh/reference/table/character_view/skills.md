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

角色须曾存在于此处，且尚未清理；角色删除后仍可取得范围。默认跳过已移除的技能，`SkipErased = false` 保留这些技能供读取。范围中的实体均为只读视图，访问对象的存活条件见[实体的身份与访问](../entity_access.md)。
