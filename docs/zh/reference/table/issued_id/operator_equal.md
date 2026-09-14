[givm](../../../reference.md) / [牌桌](../../table.md) / [issued_id](../issued_id.md) / **operator==**

# givm::operator== (issued_id)

定义于头文件 `<givm/table.hpp>`

```cpp
friend constexpr bool operator==(issued_id, issued_id) noexcept = default;
```

比较同一类别的两个已发行 ID。`operator!=` 由相等比较重写得到。

## 返回值

两个 ID 的值相等时返回 `true`，否则返回 `false`。两个无效 ID 相等。

## 注意

比较定义或标签身份时，两个 ID 须来自配套的映射；比较不会检查所属定义库。

## 示例

```cpp
#include <print>
#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    const auto first = ids.add<givm::card_definition>("恢复药剂", {});
    const auto again = ids.get_id<givm::card_definition>("恢复药剂");
    std::println("采用同一定义: {}", first == again);
}
```

输出

```text
采用同一定义: true
```
