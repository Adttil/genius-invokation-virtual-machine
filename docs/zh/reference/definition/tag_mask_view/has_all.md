# givm::tag_mask_view::has_all

定义于头文件 `<givm/definition.hpp>`

```cpp
bool has_all(std::span<const tag_id> tags) const;
```

所有指定标签都可见且已设置时返回true；空列表返回true。
