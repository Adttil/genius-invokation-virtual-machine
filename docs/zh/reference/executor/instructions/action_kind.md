[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **action_kind**

# givm::action_kind

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
enum class action_kind : std::uint8_t
{
    switch_active,
    declare_round_end
};
```

玩家在行动阶段可以提交的行动种类。

## 枚举值

| | |
| --- | --- |
| `switch_active` | 主动切换出战角色的行动 |
| `declare_round_end` | 宣布本回合结束的行动 |

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::action_request request{ .request_kind = givm::action_request_kind::do_action, .action_kind = givm::action_kind::declare_round_end };
    std::println("宣布结束: {}", request.action_kind == givm::action_kind::declare_round_end);
}
```

输出

```text
宣布结束: true
```
