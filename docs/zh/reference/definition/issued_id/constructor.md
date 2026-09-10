[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id](../issued_id.md) / **(构造函数)**

# givm::issued_id::issued_id

定义于头文件 `<givm/definition.hpp>`

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
    givm::definition_id<givm::card_definition> card{};
    std::println("已选择卡牌定义: {}", card.is_valid());
}
```

输出

```text
已选择卡牌定义: false
```
