[givm](../../reference.md) / [执行](../executor.md) / **program_input_error**

# givm::program_input_error

定义于头文件 `<givm/runtime.hpp>`

```cpp
class program_input_error;
```

响应提交程序时违反入口或输入协议的调试异常，直接公开继承 `std::exception`。它保存可读取的结构化原因，`what()` 提供同一错误的文本。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | `const std::optional<definition_name>` | 入口所属定义的类别与名称；不能确定入口时为空 |
| `program_index` | `const std::optional<std::size_t>` | 该定义通过 `add_normal_effect` 登记的响应程序编号，从零开始；不能确定入口时为空 |
| `reason` | `const program_input_error_reason` | [入口或输入协议的具体错误](program_input_error_reason.md) |

## 成员函数

| | |
| --- | --- |
| `const char* what() const noexcept override` | 取得错误文本；指针在异常对象存活且未被销毁期间有效 |

## 构造

```cpp
program_input_error(program_input_error_reason cause,
    std::optional<definition_name> definition = {}, std::optional<std::size_t> program = {});
```

以具体原因及可用的定义、程序位置构造异常。

## 注意

未定义 `NDEBUG` 时，[`invoke`](handle_context/invoke.md) 在写入本次输入前检查协议并可能抛出本异常。发布构建不进行这些检查，也不保留相关诊断元数据；违反协议仍属于未定义行为。异常类型本身在两种构建模式下均可使用。

输入尚未写入不代表整个响应或 [`resume`](execution_view/resume.md) 已回滚：此前的事件修改、命令及嵌套响应可能已经生效。捕获异常仅用于报告定义错误，不应在原执行现场继续推进。

## 参阅

| | |
| --- | --- |
| [`program_input_error_reason`](program_input_error_reason.md) | 程序提交错误的原因类型 |
| [`error_string`](error_string.md) | 将异常转换为文本 |
