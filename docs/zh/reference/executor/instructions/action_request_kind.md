[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **action_request_kind**

# givm::action_request_kind

定义于头文件 `<givm/executor/instructions/begin_action.hpp>`

```cpp
enum class action_request_kind : std::uint8_t
{
    none,
    calculate_cost,
    do_action_with_cost,
    do_action
};
```

行动请求的处理方式。

## 枚举值

| | |
| --- | --- |
| `none` | 尚未提交请求的等待状态 |
| `calculate_cost` | 仅计算选定行动费用的请求；处理后仍等待输入 |
| `do_action_with_cost` | 按此前计算的费用执行行动的请求 |
| `do_action` | 计算费用并执行行动的请求 |

## 注意

切换行动的 `calculate_cost` 请求处理完毕后，执行器再次等待输入，并将请求清空。要继续该行动，须重新提交请求，填写相同的行动种类与序号，并将 `request_kind` 设为 `do_action_with_cost`。已计算的费用用于这次尚未执行的行动；完成行动后不能沿用到下一次行动。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::action_request request{};
    request.request_kind = givm::action_request_kind::calculate_cost;
    std::println("只计算费用: {}", request.request_kind == givm::action_request_kind::calculate_cost);
}
```

输出

```text
只计算费用: true
```
