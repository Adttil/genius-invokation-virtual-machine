[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **null**

# givm::program_entry::null

定义于头文件 `<givm/definition.hpp>`

```cpp
[[nodiscard]] static constexpr program_entry null() noexcept;
```

取得一个空入口，表示尚未登记需要执行的效果。

## 返回值

空的 `program_entry`。

## 注意

空入口不能传给 `invoke`。响应没有后续效果时返回空入口（`return {};`）。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const auto entry = givm::program_entry::null();
    std::println("入口为空: {}", entry.is_null());
}
```

输出

```text
入口为空: true
```
