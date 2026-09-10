[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id](../issued_id.md) / **value**

# givm::issued_id::value

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr size_t value() const noexcept;
```

取得 ID 的数值，便于比较同一类别、同一映射中发放的 ID。

## 返回值

ID 的值。无效 ID 返回 [`invalid_value`](invalid_value.md)。

## 注意

值相同不代表不同定义库中的定义相同，不应将该值当作跨定义库的持久标识。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    const auto first = ids.add<givm::card_definition>("恢复药剂", {});
    const auto again = ids.get_id<givm::card_definition>("恢复药剂");
    std::println("指向同一定义: {}", first.value() == again.value());
}
```

输出

```text
指向同一定义: true
```
