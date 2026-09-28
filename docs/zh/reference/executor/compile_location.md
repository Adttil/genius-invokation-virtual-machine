[givm](../../reference.md) / [执行](../executor.md) / **compile_location**

# givm::compile_location

定义于头文件 `<givm/executor.hpp>`

```cpp
struct compile_location;
```

编译诊断发生的位置。与当前错误无关的位置成员为空。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `stage` | [`compile_stage`](compile_stage.md) | 编译阶段 |
| `source` | `std::optional<definition_name>` | 当前定义源的类别及名称 |
| `program` | `std::optional<program_kind>` | 初始化、回合或响应程序 |
| `program_index` | `std::optional<std::size_t>` | 当前源内响应程序的编号，从零开始 |
| `command_index` | `std::optional<std::size_t>` | 程序内命令的下标，从零开始 |

[`definition_name`](../definition/definition_name.md)、[`program_kind`](program_kind.md)

## 注意

命令位置对应定义源或调用方提交的公开命令序列，不是内部指令位置。根程序没有所属定义源。
