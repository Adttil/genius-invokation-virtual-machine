[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **stack**

# givm::execution_context::stack

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr auto& stack(this auto& self) noexcept;
```

访问本次结算使用的临时数据。

## 模板参数

|  |  |
| --- | --- |
| `self` 的推导类型 | 调用对象类型，保留 cv 限定 |

## 参数

|  |  |
| --- | --- |
| `self` | 本次执行上下文 |

## 返回值

所持有的 [`frame_stack`](../../utils/frame_stack.md) 的引用，保留对象的 cv 限定。

## 注意

指令彻底结束时，应恢复进入前的栈形状；跨调用保存的数据由该指令自己管理。外部输入仅按相应指令约定的输入槽访问。

## 示例

```cpp
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

struct effect
{
    using context_type = void;

    bool execute(givm::card_table& table, givm::execution_context& context, givm::random_fn&) const
    {
        context.stack().push(3);
        table.state().round_number = context.stack().top<int>().get<0>();
        context.stack().pop<int>();
        return context.enter_next();
    }
};

int main()
{
    givm::definition_source_library sources{};
    const auto [library, ids] = sources.compile(
        std::tuple{ effect{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }
    );
    givm::card_table table{ library };
    givm::executor execution{};
    execution.enter_entry(library);
    auto random = []() -> std::uint32_t { return 0; };
    const bool continued = execution.execute_next(table, random);
    std::println("从临时数据取得的值: {}", table.state().round_number);
}
```

输出

```text
从临时数据取得的值: 3
```
