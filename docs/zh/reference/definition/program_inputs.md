[givm](../../reference.md) / [定义](../definition.md) / **program_inputs**

# givm::program_inputs

定义于头文件 `<givm/definition.hpp>`

```cpp
class program_inputs;
```

保存一次程序调用已经准备好的全部参数，拥有其中的命令数组和嵌套延迟参数。它适合在效果准备后继续保存这份快照，或由脚本适配器逐项准备，再统一提交。

## 成员函数

| | |
| --- | --- |
| [构造函数](program_inputs/constructor.md) | 创建空输入或复制、移动已有输入 |
| [`operator=`](program_inputs/operator_assign.md) | 复制或移动另一份输入 |
| [`bytes`](program_inputs/bytes.md) | 取得已打包参数的只读字节视图 |

## 注意

通过 [`pack_inputs`](pack_inputs.md) 准备专用输入对象，通过 [`concat_inputs`](concat_inputs.md) 合并已有片段。普通 C++ 响应仍可直接逐项调用 [`invoke`](../executor/handle_context/invoke.md)，无需先构造本类型。

复制本对象会复制参数快照；移动可转移其所有权。未定义 `NDEBUG` 时，参数的类型与嵌套关系诊断信息随对象一起保存，不属于 `bytes()` 返回的内容。

## 参阅

| | |
| --- | --- |
| [`defer_invoke`](defer_invoke.md) | 为已准备的参数指定延迟目标入口 |
| [`handle_context::invoke`](../executor/handle_context/invoke.md) | 提交入口及其全部参数 |
