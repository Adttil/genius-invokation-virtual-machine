[givm](../reference.md) / **基础定义源**

# 基础定义源

定义于头文件 `<givm/basic_definitions.hpp>`，也可通过 `<givm/givm.hpp>` 使用。

`givm::genshin_impact` 提供原神各版本的草原核、激化领域和燃烧烈焰规则，供调用方选择绽放、激化和燃烧所关联的实体。它们实现了增伤、次数消耗、重复生成和耗尽离场，可以直接用于[构造定义源库](definition/definition_source_library/constructor.md)。

## 定义源对象

| 对象 | 定义名称 | 类别 |
| --- | --- | --- |
| `dendro_core_3_3_0` | `dendro_core-3.3.0-genshin_impact` | `combat_status_view` |
| `catalyzing_field_3_3_0` | `catalyzing_field-3.3.0-genshin_impact` | `combat_status_view` |
| `catalyzing_field_3_4_0` | `catalyzing_field-3.4.0-genshin_impact` | `combat_status_view` |
| `burning_flame_3_3_0` | `burning_flame-3.3.0-genshin_impact` | `summon_view` |

这些对象具有静态生命周期，可直接传入源库。调用方可以选择激化领域的不同版本，也可以用自己的定义源替代任何一项。源库按构造参数确定其用途，不从上述名称推断用途。

版本后缀表示采用的规则版本。激化领域在 3.4 中从三次改为两次，参见 [3.4 官方更新说明](https://genshin.hoyoverse.com/en/news/detail/105081)及其[完整转载](https://www.gematsu.com/2023/01/genshin-impact-version-3-4-update-now-available)。

## 效果与状态

| 定义 | 效果 | 状态上限与重复生成 |
| --- | --- | --- |
| 草原核 3.3 | 己方来源对对方出战角色造成火或雷伤害时，伤害增加 2，并消耗一层 | `count = 1`；重复生成按本次请求刷新状态 |
| 激化领域 3.3 | 己方来源对对方出战角色造成雷或草伤害时，伤害增加 1，并消耗一层 | `count = 3`；重复生成按本次请求刷新状态 |
| 激化领域 3.4 | 增伤条件与 3.3 相同 | `count = 2`；重复生成按本次请求刷新状态 |
| 燃烧烈焰 3.3 | 回合结束时，对对方出战角色造成 `value` 点火伤害，随后消耗一次可用次数 | `value = 1`、`usages = 2`；重复召唤累加本次请求的可用次数，最多两次 |

三类效果的伤害元素、增幅和使用次数见[游戏文本资料](https://gensh.honeyhunterworld.com/i_n333013/?lang=EN)。草原核和激化领域只增强对方出战角色受到的伤害，不增强对后台角色或己方角色的伤害；来源可以是己方角色、技能、召唤物等。

出战状态通过 [combat_status_state_limit](definition/queries/combat_status_state_limit.md) 提供表中的层数上限，其 `round_usages` 上限为零。每次增伤后通过状态修改命令扣层，层数归零时响应自身的状态变化并移除。重复生成使用本次已经裁剪的状态，按完整状态覆盖，不累加层数。

燃烧烈焰通过 [summon_state_limit](definition/queries/summon_state_limit.md) 限制效果量和次数。默认燃烧反应请求的状态是 `{ .value = 1, .usages = 1 }`，已有燃烧烈焰时累加一次。直接使用 [summon](definition/commands/summon.md) 或 [add_summon](definition/commands/add_summon.md) 且省略 `state`，仍遵守这些命令的规则，采用上限状态 `{ .value = 1, .usages = 2 }`。

燃烧烈焰的回合结束伤害完整结算后才扣除次数；次数耗尽时，状态修改命令立即移除召唤物并发送 [summon_removed](definition/events/summon_removed.md)。草原核和激化领域的移除发送 [combat_status_removed](definition/events/combat_status_removed.md)。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    const auto [library, ids] = compile(
        sources, std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal);
    std::println("激化领域定义: {}", library.name(library.catalyzing_field_id()));
}
```

输出

```text
激化领域定义: catalyzing_field-3.4.0-genshin_impact
```
