# givm::small_bitset::reference::operator=

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr reference& operator=(bool value) noexcept;
constexpr reference& operator=(const reference& other) noexcept;
```

将所引用位设为 `value`，或读取 `other` 当前引用的位并写入该值。返回 `*this`。

代理之间的赋值修改位值，不重新绑定目标代理。`flags[i] = flags[j]` 复制一位的值；保存的代理在赋值后仍引用原来的位。
