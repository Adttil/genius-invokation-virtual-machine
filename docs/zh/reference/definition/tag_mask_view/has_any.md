# givm::tag_mask_view::has_any

定义于头文件 `<givm/definition.hpp>`

```cpp
bool has_any(std::span<const tag_id> tags) const;
```

至少一个指定标签可见且已设置时返回true；空列表返回false。
