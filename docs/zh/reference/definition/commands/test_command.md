[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **test_command**

# givm::test_command

定义于头文件 `<givm/definition.hpp>`

```cpp
struct test_command;
```

事件响应的测试命令，用于检查实体定义中的事件响应是否能被调用。

## 注意

发出 [`test_event`](../events/test_event.md) 并等待相应效果完成后继续。

## 示例

```cpp
#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct observer_source
{
    using definition_category = givm::skill_view;
    struct definition_type { int* count; };
    int* count;
    std::string_view name() const { return "observer"; }
    definition_type compile(givm::definition_compile_context&) const { return { count }; }
    static givm::program_entry handle(
        const definition_type& data, const givm::skill_view&,
        givm::test_event&, givm::handle_context& context)
    {
        ++*data.count;
        return {};
    }
};

struct character_source
{
    using definition_category = givm::character_view;
    using definition_type = givm::definition_id<givm::skill_view>;
    std::string_view name() const { return "character"; }
    auto skill_dependencies() const { return std::array<std::string_view, 1>{ "observer" }; }
    definition_type compile(givm::definition_compile_context& context) const
    {
        return context.resolve_id<givm::skill_view>("observer");
    }
    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .health = 10 };
    }
    static definition_type query(const definition_type& skill, const givm::character_initial_skill& query)
    {
        return query.skill_index == 0 ? skill : definition_type{};
    }
};

int main()
{
    int count = 0;
    observer_source source{ &count };
    character_source character{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    sources.add(character);
    const auto [library, ids] = compile(
        sources,
        std::tuple{ givm::test_command{} },
        std::tuple{ givm::start_round{ .max_rounds = 0 } }, givm::compile_mode::normal);
    givm::table table{};
    load_deck(table, library, givm::linked_deck{
        .characters = { ids.get_id<givm::character_view>("character") }
    }, {});
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
