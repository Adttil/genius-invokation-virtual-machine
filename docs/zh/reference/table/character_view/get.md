[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **get**

# givm::character_view::get

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr attachment_view get(equipment_type type) const;
```

取得该角色指定类别的当前装备。例如，`get(equipment_type::weapon)` 取得当前武器。

## 参数

|  |  |
| --- | --- |
| `type` | 要访问的有效 [`equipment_type`](../../enums/equipment_type.md)，不能为 `none` |

## 返回值

该装备的 [`attachment_view`](../attachment_view.md)，其定义具有与 `type` 对应的装备类别标签。

## 注意

`type` 必须是有效装备类别，且调用时 [`has(type)`](has.md) 必须为 `true`；否则行为未定义。装备仍然属于角色的 [`attachments()`](attachments.md) 遍历范围。

访问对象与 ID 的保存期限见[实体的身份与访问](../entity_access.md)。
