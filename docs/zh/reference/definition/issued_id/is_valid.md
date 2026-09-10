[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id](../issued_id.md) / **is_valid**

# givm::issued_id::is_valid

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr bool is_valid() const noexcept;
```

检查是否已取得定义或标签的 ID。

## 返回值

持有 ID 时返回 `true`，否则返回 `false`。

## 注意

本函数不验证 ID 与某个定义库的对应关系。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    auto card = ids.add<givm::card_definition>("恢复药剂", {});
    std::println("已选择卡牌定义: {}", card.is_valid());
    card.set_invalid();
    std::println("清除后已选择: {}", card.is_valid());
}
```

输出

```text
已选择卡牌定义: true
清除后已选择: false
```
