[givm](../../reference.md) / [定义](../definition.md) / **concat_inputs**

# givm::concat_inputs

定义于头文件 `<givm/definition.hpp>`

```cpp
program_inputs concat_inputs(std::span<const program_inputs> parts);
```

将分别准备的输入片段合并为一次程序调用的完整参数。片段可以包含不同类型、不同数量的命令输入，数组中的元素都使用同一个 [`program_inputs`](program_inputs.md) 类型保存。

## 参数

| | |
| --- | --- |
| `parts` | 按目标程序的输入执行顺序排列的片段；空片段不贡献输入 |

## 返回值

拥有全部片段内容的 `program_inputs`。例如片段依次表示输入 A、输入 B 和 C，结果仍按 A、B、C 的顺序提交。未定义 `NDEBUG` 时，配套诊断信息也按相同顺序合并。

## 注意

脚本适配器可以在识别每项输入类型后调用 [`pack_inputs`](pack_inputs.md)，将结果追加到 `std::vector<program_inputs>`，最后一次合并。输入顺序由本函数保持，调用方不需要了解内部字节排列。

本函数完成复制后，原片段可以销毁。返回值可交给 [`invoke`](../executor/handle_context/invoke.md)，也可交给 [`defer_invoke`](defer_invoke.md) 指定延迟执行的目标。
