[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **enter_character**

# givm::enter_character

定义于头文件 `<givm/definition.hpp>`

```cpp
struct enter_character;
```

角色入场命令，指定加入哪一方队伍的角色，并按其定义准备初始状态与技能。

## 成员类型

| | |
| --- | --- |
| [`error_type`](#编译检查) | `enter_character_error` 的别名，即本命令的编译检查错误类型 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../../table/player_id.md) | 角色所属的玩家 |
| `definition` | `optional_definition_id<givm::definition_category::character>` | 要入场的角色定义 |

## 编译检查

```cpp
struct enter_character_error;
```

`enter_character::error_type` 是 `givm::enter_character_error` 的别名。`enter_character_error` 是本命令的结构化编译错误，`enter_character_error::reason` 是原因枚举。[`compile`](../../executor/compile.md) 根据本次定义集合与程序种类检查以下条件，并收集相应的结构化错误。

### 错误原因

| | |
| --- | --- |
| `invalid_player` | `player.index()` 不是固定席位 `0` 或 `1` |
| `invalid_definition` | `definition` 的定义 ID 数值超出本次编译集合的 `character_view` 定义数量 |

### `enter_character_error` 的成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `cause` | `reason` | 上表中的错误原因 |
| `value` | `std::uint64_t` | 出错的 `player.index()` 或定义 ID 的 `value()` |
| `limit` | `std::size_t` | `invalid_definition` 对应类别的定义数量，即有效 ID 数值范围的上界（不含） |

仅与当前 `cause` 对应的附加成员具有诊断含义。

## 注意

新角色使用定义库已保存的 [`character_initial_state`](../queries/character_initial_state.md) 结果作为初始状态，再从零开始逐项查询 [`character_initial_skill`](../queries/character_initial_skill.md)，直到首次得到无效 ID，并以默认技能状态加入角色。入场不自动将其设为出战角色。

## 示例

```cpp
#include <array>
#include <cstdint>
#include <print>
#include <string_view>
#include <tuple>
#include <utility>

#include <givm/givm.hpp>

struct character_source
{
    static constexpr auto category = givm::definition_category::character;
    struct definition_type {};
    std::string_view name() const { return "character"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }

    static givm::character_state query(const definition_type&, const givm::character_initial_state&)
    {
        return { .max_health = 10, .max_energy = 3, .health = 10, .energy = 0 };
    }
};

struct effect_source
{
    static constexpr auto category = givm::definition_category::card;
    std::string_view name() const { return "effect"; }
    auto character_dependencies() const
    { return std::array<std::string_view, 1>{ "character" }; }

    givm::normal_effect compile(givm::definition_compile_context& context) const
    {
        const auto definition = context.resolve_id<givm::definition_category::character>("character");
        return context.add_normal_effect(
            givm::enter_character{ .player = givm::player_id{ 0 }, .definition = definition });
    }

    static givm::normal_effect handle(const givm::normal_effect& entry,
        givm::round_started&, givm::handle_context<givm::deck_card_view>& context, std::uint32_t = 0)
    {
        return context.invoke(entry);
    }
};

int main()
{
    const character_source source{};
    const effect_source effect{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source, effect)) return 1;
    auto library_result = compile(sources, basics, std::tuple{},
        std::tuple{
            givm::start_round{}, givm::settle{}, givm::end_game{ .result = givm::game_result::both_loss }
        }, givm::compile_mode::normal);
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    givm::table table{};
    load_deck(table, library,
        givm::linked_deck{ .cards = { ids.get_id<givm::definition_category::card>("effect") } },
        givm::linked_deck{});
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    const auto initialized = execution.start(library, table);
    initialized.resume(library, table, random);
    const auto character = table[givm::character_id{ givm::player_id{ 0 }, 0  }];
    std::println("初始生命: {}", character.state().health);
}
```

输出

```text
初始生命: 10
```

## 参阅

| | |
| --- | --- |
| [`character_initial_state`](../queries/character_initial_state.md) | 角色初始状态查询 |
| [`character_initial_skill`](../queries/character_initial_skill.md) | 角色初始技能查询 |
