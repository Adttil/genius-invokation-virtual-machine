[givm](../../../reference.md) / [定义](../../definition.md) / [program_inputs](../program_inputs.md) / **bytes**

# givm::program_inputs::bytes

定义于头文件 `<givm/definition.hpp>`

```cpp
std::span<const unsigned char> bytes() const noexcept;
```

取得本对象已打包参数的只读字节视图。该视图借用本对象的数据，不包含 Debug 诊断信息。

## 注意

向程序提交参数时传递整个 [`program_inputs`](../program_inputs.md) 对象，以保留配套诊断信息。字节内容使用当前构建的原生表示，不是跨进程或跨版本的序列化格式。

所引用的参数缓冲区被销毁或替换后，视图失效；移动操作将缓冲区的所有权交给目标对象。
