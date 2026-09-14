[givm](../../../reference.md) / [牌桌](../../table.md) / [issued_id](../issued_id.md) / **value**

# givm::issued_id::value

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr size_t value() const noexcept;
```

取得 ID 的数值，便于比较同一类别、同一映射中发放的 ID。

## 返回值

ID 的值。无效 ID 返回 [`invalid_value`](invalid_value.md)。

## 注意

值相同不代表不同映射中的标签相同，不应将该值当作跨定义库的持久标识。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    const auto first = ids.get_tag_id("治疗");
    const auto again = ids.get_tag_id("治疗");
    std::println("指向同一标签: {}", first.value() == again.value());
}
```

输出

```text
指向同一标签: true
```
