[givm](../../../reference.md) / [牌桌](../../table.md) / [character_view](../character_view.md) / **has**

# givm::character_view::has

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr bool has(equipment_type type) const;
```

判断该角色当前是否具有指定类别的装备。例如，`has(equipment_type::weapon)` 判断角色是否装备武器。

## 参数

|  |  |
| --- | --- |
| `type` | 要查询的有效 [`equipment_type`](../../enums/equipment_type.md)，不能为 `none` |

## 返回值

存在该类别尚未移除的装备时返回 `true`，否则返回 `false`。

## 注意

传入 `none` 或其他无效枚举值时，行为未定义。返回 `true` 时，可以使用 [`get(type)`](get.md) 取得该装备。装备移除后，本接口返回 `false`；原装备实体在清理前仍遵循[实体的身份与访问](../entity_access.md)的读取约定。
