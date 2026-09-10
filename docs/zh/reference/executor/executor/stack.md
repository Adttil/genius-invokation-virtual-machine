[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **stack**

# givm::executor::stack

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr auto& stack(this auto& self) noexcept;
```

访问结算中的临时数据，例如当前指令约定的外部输入槽。

## 模板参数

| | |
| --- | --- |
| `self` 的推导类型 | 由调用对象推导，保留其 cv 限定 |

## 参数

| | |
| --- | --- |
| `self` | 要访问其执行栈的执行器 |

## 返回值

所持有的 [`frame_stack`](../../utils/frame_stack.md) 的左值引用，保留调用对象的 cv 限定。

## 注意

访问外部输入槽时，应遵守具体指令约定的条件、类型和顺序。

## 示例

```cpp
#include <print>
#include <utility>

#include <givm/givm.hpp>

int main()
{
    givm::executor execution{};
    auto& writable = execution.stack();
    const auto& readable = std::as_const(execution).stack();
    std::println("两种访问指向同一个栈: {}", &writable == &readable);
    std::println("初始临时数据为空: {}", readable.empty());
}
```

输出

```text
两种访问指向同一个栈: true
初始临时数据为空: true
```

## 参阅

| | |
| --- | --- |
| [`clear`](clear.md) | 清空执行栈 |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
