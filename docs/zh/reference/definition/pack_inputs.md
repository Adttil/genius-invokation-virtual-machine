[givm](../../reference.md) / [定义](../definition.md) / **pack_inputs**

# givm::pack_inputs

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class... T> // 每个 T 均须为核心命令声明的 input_type
program_inputs pack_inputs(const T&... inputs);
```

将一次效果所需的专用输入对象保存为拥有型参数快照。按目标程序的动态命令执行顺序提供参数，固定命令不占输入位置。

## 返回值

拥有全部输入、命令数组和嵌套延迟参数的 [`program_inputs`](program_inputs.md)。不传参数时返回空输入。

## 注意

本函数返回后，原输入对象和数组可以销毁。它不执行程序，也不访问定义库；输入是否与程序匹配，在提交或编译固定延迟命令时检查。

普通响应直接调用 [`invoke(entry, inputs...)`](../executor/handle_context/invoke.md) 即可。需要事先保存参数时使用本函数；脚本适配器可为每个已识别的具体输入类型分别调用，再通过 [`concat_inputs`](concat_inputs.md) 按顺序合并，无须构造输入 variant。

未定义 `NDEBUG` 时，类型与嵌套关系诊断信息随结果保存；这些信息不写入执行参数字节。
