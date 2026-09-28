[givm](../../reference.md) / [执行](../executor.md) / **command_input_error**

# givm::command_input_error

定义于头文件 `<givm/executor.hpp>`

```cpp
class command_input_error;
```

命令执行时检测到输入值或执行前提不合法的调试异常，直接公开继承 `std::exception`。例如目标实体越界或已离场、输入列表包含不允许重复的实体、请求移除的骰子不足，或响应把伤害倍率分母改为零。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `command` | `const std::string` | 检测到错误的命令名称 |
| `reason` | `const command_input_error_reason` | [输入值或执行前提的具体错误](command_input_error_reason.md) |

## 成员函数

| | |
| --- | --- |
| `const char* what() const noexcept override` | 取得包含命令名称和具体原因的错误文本 |

## 构造

```cpp
command_input_error(std::string_view command, command_input_error_reason reason);
```

以命令名称和具体原因构造异常。异常保存名称，不借用传入字符。

## 检查时点

[`invoke`](handle_context/invoke.md) 检查入口与输入对象的匹配关系；命令的值与牌桌前提在执行该命令时检查。此前命令可能已经改变牌桌，因此不能以提交时的实体或资源状态代替执行时状态。例如先产骰再移除这些骰子的程序可以一次提交全部输入，即使提交时还没有这些骰子。

允许被响应修改的事件，在随后使用相关字段前检查必要条件。例如伤害初始分母合法，但数值响应把它改为零，仍会报告 `invalid_numeric_argument`。命令本来允许的饱和加减、空批次或找不到目标时跳过等规则，不因此变成错误。

## 注意

这些诊断仅在未定义 `NDEBUG` 时检查和抛出。发布构建不保留检查、对应诊断存储或错误分支中的显式抛出；定义源仍须满足各命令的输入约定。异常类型本身在两种构建模式下均可使用。

不保证命令、响应或整次 [`step`](executor/step.md) 回滚。前序命令及嵌套效果可能已生效，捕获异常后不应在原执行现场继续推进。本机制用于定位定义错误，不代替行动视图面向上层的独立合法性检查。

## 参阅

| | |
| --- | --- |
| [`program_input_error`](program_input_error.md) | 程序提交协议的调试异常 |
| [`error_string`](error_string.md) | 将运行时诊断转换为文本 |
