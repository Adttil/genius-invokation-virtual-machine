# givm::small_bitset::reference::reference

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr reference(const reference&) noexcept;
```

复制对同一个位的借用，不复制位数据，也不延长原位集的生命周期。初始代理通过位集的 [`operator[]`](../operator_at.md) 取得。
