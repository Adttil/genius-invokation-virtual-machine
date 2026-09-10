[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **operator bool**

# givm::program_entry::operator bool

定义于头文件 `<givm/definition.hpp>`

```cpp
[[nodiscard]] constexpr explicit operator bool() const noexcept;
```

检查是否选择了后续效果或终局入口。

## 返回值

空入口返回 `false`，普通效果入口或终局入口返回 `true`。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    using entry_type = givm::handler_program_entry_t<givm::round_ended>;
    entry_type effect{};
    std::println("尚无后续效果: {}", effect.is_null());
    effect = entry_type::both_loss();
    std::println("已选择终局效果: {}", static_cast<bool>(effect));
}
```

输出

```text
尚无后续效果: true
已选择终局效果: true
```
