[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **test_command**

# givm::test_command

定义于头文件 `<givm/definition.hpp>`

```cpp
struct test_command;
```

事件响应的测试命令，用于检查实体定义中的事件响应是否能被调用。

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
    using definition_category = givm::character_view;
    struct definition_type { int* count; };
    int* count;
    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }
    static givm::program_entry<givm::test_event> handle(
        const definition_type& data, const givm::character_view&,
        givm::test_event&, const givm::table&, givm::random_fn&)
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
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::test_command{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    table.load_deck(givm::player_id{ 0 }, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("observer") }
    });
    auto random = []() -> std::uint32_t { return 0; };
    givm::executor execution{};
    execution.enter_entry(library);
    execution.step(library, table, random);
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
