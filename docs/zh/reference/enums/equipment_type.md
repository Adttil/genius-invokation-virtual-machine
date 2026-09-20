[givm](../../reference.md) / [枚举值](../enums.md) / **equipment_type**

# givm::equipment_type

定义于头文件 `<givm/enums/equipment_type.hpp>`

```cpp
enum class equipment_type : std::uint8_t
{
    weapon,
    artifact,
    talent,
    technique,
    none
};
```

附着实体定义的装备类别。定义库根据定义标签确定分类，通过 [`definition_library::equipment_type`](../executor/definition_library/equipment_type.md) 取得。角色视图的 [`has(type)`](../table/character_view/has.md) 和 [`get(type)`](../table/character_view/get.md) 使用此枚举指定要访问的装备类别。

## 枚举值

|  |  |
| --- | --- |
| `weapon` | 武器，对应 `weapon` 标签 |
| `artifact` | 圣遗物，对应 `artifact` 标签 |
| `talent` | 天赋，对应 `talent` 标签 |
| `technique` | 特技，对应 `technique` 标签 |
| `none` | 普通附属实体，没有装备类别标签 |

## 注意

四种装备类别标签互斥，由定义源保证，不进行冲突检查。装备的创建仍由定义 ID 决定类别，无须单独指定枚举值。

装备类别从零开始连续编号，`none` 始终位于最后，其底层值等于有效装备类别的数量。`none` 不能传入角色的装备访问接口。
