[givm](../../../reference.md) / [执行](../../executor.md) / [executor](../executor.md) / **start**

# givm::executor::start

定义于头文件 `<givm/executor.hpp>`

```cpp
void start(const definition_library& library, table& table);
```
[`definition_library`](../definition_library.md)、[`table`](../../table/table.md)

准备按照 `library` 提供的游戏流程开始一场对局。

原有的待完成结算被丢弃。按定义库布局准备历史摘要的状态空间，并向摘要发送 [`history_summary_initialization`](../../definition/events/history_summary_initialization.md)，全部初始化响应在本函数返回前完成。本函数不执行游戏流程中的命令；通过 [`step`](step.md) 开始推进。

## 参数

| | |
| --- | --- |
| `library` | 本场对局使用的定义库 |
| `table` | 本场对局使用的牌桌 |

## 返回值

（无）

## 注意

通常先调用 [`load_deck`](../load_deck.md) 装载双方牌组，再调用本函数，使摘要初始化响应可以读取完整的初始牌桌。摘要字段不保证清零；定义源须在读取字段前通过初始化响应或后续写入赋予有效值。重新调用本函数会开始新的执行与摘要初始化，不会重置其他牌桌状态。

每次推进时须显式传入配套的 `library` 和 `table`。本函数不保存二者的指针或引用。

## 示例

```cpp
#include <utility>
#include <print>
#include <cstdint>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    auto library_result = compile(
        sources, basics,
        std::tuple{ givm::shuffle_deck{ .player = givm::player_id{ 0 } } },
        std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{ { .max_rounds = 1 } };
    load_deck(table, library, {}, {});
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.start(library, table);
    std::println("牌桌回合数保持原值: {}", table.state().round_number);
    const auto state = execution.step(library, table, random);
    std::println("随后推进至终局: {}", state == givm::execution_state::finished);
}
```

输出

```text
牌桌回合数保持原值: 0
随后推进至终局: true
```

## 参阅

| | |
| --- | --- |
| [`step`](step.md) | 推进至编译模式要求报告的下一处现场 |
