[givm](../../../reference.md) / [定义](../../definition.md) / [program_inputs](../program_inputs.md) / **operator=**

# givm::program_inputs::operator=

定义于头文件 `<givm/definition.hpp>`

```cpp
program_inputs& operator=(const program_inputs& other);
program_inputs& operator=(program_inputs&& other) noexcept;
```

用另一份输入替换当前参数。复制赋值保留独立的快照；移动赋值转移 `other` 的参数所有权。

## 返回值

`*this`。

## 注意

赋值后不再使用此前从本对象取得的 [`bytes`](bytes.md) 视图。
