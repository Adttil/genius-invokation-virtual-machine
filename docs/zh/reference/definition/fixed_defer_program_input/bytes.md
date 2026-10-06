[givm](../../../reference.md) / [定义](../../definition.md) / [fixed_defer_program_input](../fixed_defer_program_input.md) / **bytes**

# givm::fixed_defer_program_input::bytes

```cpp
std::span<const unsigned char> bytes() const noexcept;
```

取得已打包参数的只读字节视图，不包含编译校验描述。视图借用本对象拥有的参数，对象销毁或被赋值后不能继续使用。
