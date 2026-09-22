[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **add_support**

# givm::add_support

定义于头文件 `<givm/definition.hpp>`

向一位玩家的支援区添加一个独立支援。同定义支援可以同时存在；区域已满时，本次添加无效。

```cpp
struct add_support
{
    relative_player player = relative_player::current;
    definition_id<support_view> definition{};
    support_state state{
        std::numeric_limits<std::uint32_t>::max(),
        std::numeric_limits<std::uint32_t>::max()
    };
};
```

## 输入

- 默认构造 `add_support{}` 使用动态模式，由 `invoke` 提交一个 [support_addition](../events/support_addition.md)。
- `definition` 非空时使用固定模式，不消费响应输入；目标范围为 `player` 指定的一方。

`player` 沿用 [relative_player](relative_player.md) 的含义，相对于当前行动玩家。动态输入明确指定目标玩家和定义，两者须合法。

## 结算

执行时读取 [support_state_limit](../queries/support_state_limit.md)，将提供的 `state` 各字段分别裁剪至对应上限。`state` 省略时，两个字段均为 `UINT32_MAX`，经同样的裁剪后得到该定义的上限；显式指定较小的值可以创建较少层数或次数的实体。

目标玩家的有效支援数量小于其当前 [`player_state::support_limit`](../../table/player_state.md) 时，创建独立实体。同定义实体的存在不改变本次操作，不会刷新旧实体或向其发送重复添加通知。

达到或超过上限时，不创建实体，也不移除已有支援或广播 [`support_removed`](../events/support_removed.md)。容量在本命令实际执行时判断；已移除支援不占容量，上限默认 4。

手打支援牌需要替换旧支援时，由牌定义先执行 [`remove_support`](remove_support.md)，再执行本命令。旧支援的离场广播及响应程序全部完成后才尝试添加；若空位已被其他效果占满，添加同样无效。通过其他效果生成支援时，直接使用本命令即可。

显式指定 `.state = {}` 时，两个字段均为零；部分初始化 `state` 时，省略的字段也会初始化为零。

## 示例

```cpp
#include <array>
#include <cstdint>
#include <print>
#include <ranges>
#include <string_view>
#include <tuple>

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

int main()
{
    support_source support{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(support);
    const auto issued = sources.make_issued_id_map();
    const auto id = issued.get_id<givm::support_view>("support");
    const auto [library, ids] = compile(sources,
        std::array{
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id },
            givm::add_support{ .definition = id }
        },
        std::tuple{ givm::end_game{ .result = givm::game_result::both_loss } },
        givm::compile_mode::normal);
    givm::table table{};
    givm::executor execution{};
    auto random = []() -> std::uint32_t { return 0; };
    execution.enter_entry(library);
    execution.step(library, table, random);
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
