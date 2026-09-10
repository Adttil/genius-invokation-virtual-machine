[givm](../../../reference.md) / [定义](../../definition.md) / [program_entry](../program_entry.md) / **operator== (program_entry)**

# givm::operator== (program_entry)

定义于头文件 `<givm/definition.hpp>`

```cpp
friend constexpr bool operator==(program_entry, program_entry) noexcept = default;
```

比较两个入口是否选择了相同的后续效果。

## 参数

|  |  |
| --- | --- |
| 两个操作数 | context 相同、属于同一定义库的入口，或命名的空入口与终局入口 |

## 返回值

入口相等时返回 `true`，否则返回 `false`。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    using entry_type = givm::handler_program_entry_t<givm::round_ended>;
    const auto first = entry_type::player_0_win();
    const auto second = entry_type::player_1_win();
    std::println("选择同一终局: {}", first == second);
    std::println("默认入口为空: {}", entry_type{} == entry_type::null());
}
```

输出

```text
选择同一终局: false
默认入口为空: true
```
