[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **(构造函数)**

# givm::program_entry::program_entry

定义于头文件 `<givm/definition.hpp>`

```cpp
constexpr program_entry() noexcept = default;
```

构造一个表示没有后续效果的空入口。

## 返回值

（无）

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
