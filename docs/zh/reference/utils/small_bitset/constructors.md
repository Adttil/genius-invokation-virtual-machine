# givm::small_bitset::small_bitset

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr small_bitset() noexcept;
constexpr explicit small_bitset(std::size_t size);
constexpr small_bitset(const small_bitset& other);
constexpr small_bitset(small_bitset&& other) noexcept;
```

默认构造得到空位集；指定 `size` 时创建具有这些位、所有位均为零的位集。`size` 可以超过 `inline_capacity`，此时分配整个位集所需的存储。

复制保留源的位数及内容，之后两者可独立修改。移动取得源的内容并使源变为空位集；源仍可重新赋值。
