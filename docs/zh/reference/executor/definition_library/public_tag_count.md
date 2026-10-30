# givm::definition_library::public_tag_count

定义于头文件 `<givm/runtime.hpp>`

```cpp
std::size_t public_tag_count() const noexcept;
```

返回公开标签的数量。公开标签ID位于 `[0, public_tag_count())`，其余标签ID位于后方；同一编译结果的映射和定义库采用相同顺序。
