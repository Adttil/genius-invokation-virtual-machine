# givm::bitset_view::take

定义于头文件 `<givm/utils/small_bitset.hpp>`

```cpp
constexpr bitset_view take(std::size_t count) const noexcept;
```

取得本视图前 `count` 位的借用，保留原数据地址，不复制或分配。空视图允许调用 `take(0)`。

要求 `count <= size()`。Debug 构建断言此前提，Release 不检查。返回视图与原视图共同借用同一份存储，不延长其生命周期。
