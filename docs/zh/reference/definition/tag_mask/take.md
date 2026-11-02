# givm::tag_mask::take

定义于头文件 `<givm/definition.hpp>`

```cpp
tag_mask_view take(std::size_t count) const noexcept;
```

返回前 `count` 个标签位的只读借用，不复制或分配。

要求 `count` 不超过原位集大小。Debug 构建断言此前提，Release 不检查。
