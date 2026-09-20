[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **equipment_type**

# givm::definition_library::equipment_type

定义于头文件 `<givm/executor.hpp>`

```cpp
givm::equipment_type equipment_type(definition_id<attachment_view> id) const noexcept;
```

取得附着实体定义的装备类别，用于区分武器、圣遗物、天赋、特技和普通附属实体。

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效附着实体定义 ID |

## 返回值

与定义的装备类别标签对应的 [`givm::equipment_type`](../../enums/equipment_type.md)。没有装备类别标签时返回 `none`。

## 注意

分类由定义的 `weapon`、`artifact`、`talent`、`technique` 标签确定。四个标签互斥，由定义源保证，不进行冲突检查。
