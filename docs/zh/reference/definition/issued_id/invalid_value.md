[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id](../issued_id.md) / **invalid_value**

# givm::issued_id::invalid_value

定义于头文件 `<givm/definition.hpp>`

```cpp
static constexpr size_t invalid_value = static_cast<size_t>(-1);
```

未取得有效 ID 时使用的值。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::tag_id selected{};
    std::println("尚未选择标签: {}", selected.value() == givm::tag_id::invalid_value);
}
```

输出

```text
尚未选择标签: true
```
