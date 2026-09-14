[givm](../../../reference.md) / [牌桌](../../table.md) / [issued_id](../issued_id.md) / **(构造函数)**

# givm::issued_id::issued_id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr issued_id() noexcept = default;
```

构造一个尚未关联任何定义或标签的 ID。

## 返回值

（无）

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::tag_id selected{};
    std::println("已选择标签: {}", selected.is_valid());
}
```

输出

```text
已选择标签: false
```
