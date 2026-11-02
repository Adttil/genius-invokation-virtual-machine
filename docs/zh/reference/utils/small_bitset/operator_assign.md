# givm::small_bitset::operator=

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr small_bitset& operator=(small_bitset other) noexcept;
```

将位数和内容整体替换为 `other` 的位数和内容，按新位数选择存储方式。复制后内容独立；移动后源为空。给自身复制或移动赋值均保持原值。返回 `*this`。

参数按值接收：左值在进入函数前复制，右值在进入函数前移动；随后通过 `swap` 替换内容。参数复制过程中分配失败时目标保持原值，函数体本身不抛异常。成功替换内容后，此前从目标借出的视图及位代理不再保证有效。
