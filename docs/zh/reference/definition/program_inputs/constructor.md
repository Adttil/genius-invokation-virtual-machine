[givm](../../../reference.md) / [定义](../../definition.md) / [program_inputs](../program_inputs.md) / **构造函数**

# givm::program_inputs 的构造函数

定义于头文件 `<givm/definition.hpp>`

```cpp
program_inputs() noexcept;
program_inputs(const program_inputs&);
program_inputs(program_inputs&&) noexcept;
```

默认构造得到空输入，适用于没有动态参数的程序。复制构造保留独立的参数快照；移动构造转移已有参数。

准备非空输入使用 [`pack_inputs`](../pack_inputs.md) 或 [`concat_inputs`](../concat_inputs.md)。
