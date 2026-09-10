[givm](../../reference.md) / [指令](../instructions.md) / **action_request**

# givm::action_request

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
struct action_request;
```

调用方提交给行动阶段的一次请求。它说明要选择哪种行动，以及是否先计算费用。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `request_kind` | [`action_request_kind`](action_request_kind.md) | 请求的处理方式，初始为 none |
| `action_kind` | [`action_kind`](action_kind.md) | 行动种类，初始为 switch_active |
| `action_index` | [`stack_count_t`](../utils/stack_count_t.md) | 切换目标在当前存活非出战角色中的序号，从 0 开始 |

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

## 参阅

| | |
| --- | --- |
| [`begin_action`](begin_action.md) | 提交玩家行动 |
