# givm::tag_mask::first

定义于头文件 `<givm/definition.hpp>`

```cpp
tag_mask_view first(std::size_t count) const noexcept;
```

返回前 `count` 个标签位的只读借用。`count` 必须不超过原位集大小；不执行运行期检查，也不复制位集。
