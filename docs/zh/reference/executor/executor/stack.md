[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **stack**

# givm::executor::stack

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr auto& stack(this auto& self) noexcept;
```

返回执行器持有的栈。

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
#include <givm/executor.hpp>

#include <concepts>
#include <iostream>
#include <utility>

int main()
{
    givm::executor execution{};

    auto& writable_stack = execution.stack();
    const auto& readable_stack = std::as_const(execution).stack();

    static_assert(std::same_as<decltype(writable_stack), givm::frame_stack&>);
    static_assert(std::same_as<decltype(readable_stack), const givm::frame_stack&>);

    std::cout << std::boolalpha
              << "same stack: " << (&writable_stack == &readable_stack) << '\n'
              << "stack empty: " << readable_stack.empty() << '\n';
}
```

输出

```text
same stack: true
stack empty: true
```

## 参阅

| | |
| --- | --- |
| [`clear`](clear.md) | 清空执行栈 |
| [`execute_next`](execute_next.md) | 完整执行当前位置的一次指令 |
