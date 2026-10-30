# givm::tag_mask_view::matches

定义于头文件 `<givm/definition.hpp>`

```cpp
bool matches(std::span<const tag_id> required, std::span<const tag_id> excluded = {}) const;
```

同时满足必需标签全部存在、排除标签全部不存在时返回true。
