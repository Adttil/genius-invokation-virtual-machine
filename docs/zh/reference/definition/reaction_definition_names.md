[givm](../../reference.md) / [定义](../definition.md) / **reaction_definition_names**

# givm::reaction_definition_names

定义于头文件 `<givm/definition_source_interface.hpp>`，亦由 `<givm/definition.hpp>` 和 `<givm/compile.hpp>` 提供。

编译时指定每个基础反应槽位采用的默认定义名称。它按反应枚举访问，不向调用方暴露数组下标；同一个名称可以用于多个槽位。`none` 不表示槽位，不能作为索引。

```cpp
class reaction_definition_names;
```

## 成员函数

| 名称 | 说明 |
| --- | --- |
| [构造函数](reaction_definition_names/constructor.md) | 构造空名称表，或将全部槽位设为同一名称 |
| [`operator[]`](reaction_definition_names/operator_at.md) | 读取或设置指定反应槽位的名称 |

调用方必须提前向源库 `add` 这些反应定义及其完整依赖闭包；编译不注入定义源。部分选择编译也始终包含这组默认反应及其依赖。随库默认名称表为 `genshin_impact::reaction_names_3_3_0`；`reaction_sources_3_3_0()` 返回可直接 add 的完整默认反应源库。

角色通过 `character_reaction_override` 查询声明整局替换。`load_deck` 按牌组角色顺序应用非空替换，同槽位后面的覆盖前面的，并将实际映射保存到牌桌。
