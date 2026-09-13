[givm](../../../reference.md) / [执行](../../executor.md) / [指令](../instructions.md) / **test_command**

# givm::test_command

定义于头文件 `<givm/executor/instructions/test_command.hpp>`

```cpp
struct test_command;
```

事件响应的测试指令，用于检查实体定义中的事件响应是否能被调用。

## 成员类型

| | |
| --- | --- |
| `context_type` | `void`，表示不依赖特定事件语境 |

## 注意

发出 [`test_event`](../events/test_event.md) 并等待相应效果完成后继续。

## 示例

```cpp
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct observer_source
{
    using definition_category = givm::support_view;
    struct definition_type { int* count; };
    int* count;
    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }
    static givm::program_entry<givm::test_event> handle(
        const definition_type& data, const givm::support_view&,
        givm::test_event&, const givm::card_table&, givm::random_fn&)
    {
        ++*data.count;
        return givm::program_entry<givm::test_event>::null();
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{ givm::test_command{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } });
    givm::card_table table{ library };
    table[givm::player_id{ 0 }].add(ids.get_id<givm::support_view>("observer"), {});
    table.state().round_number = 1;
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.run(table, random);
    std::println("收到事件次数: {}", count);
}
```

输出

```text
收到事件次数: 1
```

## 参阅

| | |
| --- | --- |
| [`test_event`](../events/test_event.md) | 事件响应的测试通知 |
