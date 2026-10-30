# givm::table::view_for

定义于头文件 `<givm/table.hpp>`

```cpp
template<role Role>
constexpr auto view_for() const noexcept;
```

取得借用本牌桌的 [`perspective_table_view<Role>`](../perspective_table_view.md)，按指定身份表达可读内容。牌桌本身保持现有完整只读能力。

包装不复制牌桌、不拥有数据，也不改变对局。原牌桌必须在使用期间存活。
