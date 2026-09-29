[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_support**

# givm::add_support

定义于头文件 `<givm/definition.hpp>`

向一位玩家的支援区添加一个独立支援。同定义支援可以同时存在；区域已满时，本次添加无效。

```cpp
struct add_support_error;

struct add_support
{
    using error_type = add_support_error;

    using input_type = add_support_input;

    relative_player player = relative_player::self;
    definition_id<support_view> definition{};
    support_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 成员类型

| | |
| --- | --- |
| `input_type` | [`add_support_input`](../command_inputs/add_support_input.md)，动态模式下的输入类型 |
| [`error_type`](#编译检查) | `add_support_error` 的别名，即本命令的编译检查错误类型 |

## 输入

- 默认构造 `add_support{}` 使用动态模式，由 `invoke` 提交一个 [add_support_input](../command_inputs/add_support_input.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前效果的本方。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [support_state_limit](../queries/support_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

目标玩家的有效支援数量小于其当前 [`player_state::support_limit`](../../table/player_state.md) 时，创建独立实体。同定义实体的存在不改变本次操作，不会刷新旧实体或向其发送重复添加通知。

达到或超过上限时，不创建实体，也不移除已有支援或广播 [`support_removed`](../events/support_removed.md)。容量在本命令实际执行时判断；已移除支援不占容量，上限默认 4。

手打支援牌需要替换旧支援时，由牌定义先执行 [`remove_support`](remove_support.md)，再执行本命令。旧支援的离场广播及响应程序全部完成后才尝试添加；若空位已被其他效果占满，添加同样无效。通过其他效果生成支援时，直接使用本命令即可。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。

## 编译检查

```cpp
struct add_support_error;
```

`add_support::error_type` 是 `givm::add_support_error` 的别名。`add_support_error` 是本命令的结构化编译错误，`add_support_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `dynamic_input_in_root` | 初始化或回合根流程使用了动态输入模式；该模式只允许出现在响应程序中 |
| `invalid_player` | `player` 不是 `relative_player::self` 或 `relative_player::opponent` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `support_view` 定义数量 |

### `add_support_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::size_t` | 出错字段的数值；定义 ID 使用其 `value()`，枚举使用其底层数值 |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。`dynamic_input_in_root` 不使用附加成员；动态模式不检查未使用的固定参数。

## 示例

```cpp
#include <array>
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/givm.hpp>

struct support_source
{
    using definition_category = givm::support_view;
    struct definition_type {};
    std::string_view name() const { return "support"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::support_state query(const definition_type&, const givm::support_state_limit&)
    {
        return { .count = 3, .round_usages = 2 };
    }
};

struct effect_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "effect"; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "support" }; }

    givm::program_entry compile(givm::definition_compile_context& context) const
    {
        const auto id = context.resolve_id<givm::support_view>("support");
        return context.add_program(
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id });
    }

    static givm::program_entry handle(const givm::program_entry& entry, const givm::deck_card_view&,
        givm::round_started&, givm::handle_context& context)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const support_source support{};
    const effect_source effect{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0,
        givm::genshin_impact::shield_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(support, effect)) return 1;
    auto library_result = compile(sources, basics, std::tuple{},
        std::tuple{
            givm::start_round{},
            givm::end_game{ .result = givm::game_result::both_loss }
        }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{};
    load_deck(table, library,
        givm::linked_deck{ .cards = { ids.get_id<givm::card_definition>("effect") } },
        givm::linked_deck{});
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    auto supports = table[givm::player_id{ 0 }].supports();
    const auto first = *supports.begin();
    std::println("支援数量: {}", std::ranges::distance(supports));
    std::println("首个支援的层数: {}", first.state().count);
    std::println("首个支援的回合次数: {}", first.state().round_usages);
}
```

输出

```text
支援数量: 4
首个支援的层数: 3
首个支援的回合次数: 2
```
