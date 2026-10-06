[givm](../../reference.md) / [执行](../executor.md) / **effect_command_not_allowed**

# givm::effect_command_not_allowed

定义于头文件 `<givm/compile.hpp>`。

```cpp
struct effect_command_not_allowed
{
    event_category category;
    std::string_view command;
};
```

效果包含其类别不允许执行的命令。目前用于立即效果中的 `end_segment` 和 `settle`：立即效果的记录属于外层段，不能自行分段或结算。

| 成员 | 说明 |
| --- | --- |
| `category` | 不允许执行该命令的效果类别，目前为 `immediate` |
| `command` | 命令名称 |

错误随整库 [`compile`](compile.md) 返回，位置给出定义源、效果编号与命令下标。Debug 和 Release 都进行此项编译检查。
