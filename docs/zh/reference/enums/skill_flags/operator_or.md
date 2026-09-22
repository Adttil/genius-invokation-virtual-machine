[givm](../../../reference.md) / [skill_flags](../skill_flags.md) / **operator_or**

# operator_or

```cpp
skill_flags operator|(skill_flags lhs, skill_flag_bits rhs) noexcept;
skill_flags operator|(skill_flag_bits lhs, skill_flag_bits rhs) noexcept;
```

所有函数均为 `constexpr`。返回同时包含两边性质的集合。
