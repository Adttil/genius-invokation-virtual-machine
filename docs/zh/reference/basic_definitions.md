[givm](../reference.md) / **基础定义源**

# 基础定义源

定义于头文件 `<givm/basic_definitions.hpp>`，也可通过 `<givm/givm.hpp>` 使用。

`givm::genshin_impact` 提供原神各版本的草原核、激化领域、燃烧烈焰和冻结规则，供调用方选择元素反应所关联的实体。它们实现各自的增伤、消耗和离场效果，可以直接用于[构造定义源库](definition/definition_source_library/constructor.md)。

## 定义源对象

| 对象 | 定义名称 | 类别 |
| --- | --- | --- |
| `dendro_core_3_3_0` | `dendro_core-3.3.0-genshin_impact` | `combat_status_view` |
| `catalyzing_field_3_3_0` | `catalyzing_field-3.3.0-genshin_impact` | `combat_status_view` |
| `catalyzing_field_3_4_0` | `catalyzing_field-3.4.0-genshin_impact` | `combat_status_view` |
| `burning_flame_3_3_0` | `burning_flame-3.3.0-genshin_impact` | `summon_view` |
| `frozen_3_3_0` | `frozen-3.3.0-genshin_impact` | `attachment_view` |

这些对象具有静态生命周期，可直接传入源库。调用方可以选择激化领域的不同版本，也可以用自己的定义源替代任何一项。源库按构造参数确定其用途，不从上述名称推断用途。

版本后缀表示采用的规则版本。激化领域在 3.4 中从三次改为两次，参见 [3.4 官方更新说明](https://genshin.hoyoverse.com/en/news/detail/105081)及其[完整转载](https://www.gematsu.com/2023/01/genshin-impact-version-3-4-update-now-available)。

## 效果与状态

| 定义 | 效果 | 状态上限与重复生成 |
| --- | --- | --- |
| 草原核 3.3 | 己方来源对对方出战角色造成火或雷伤害时，伤害增加 2，并消耗一层 | `count = 1`；重复生成按本次请求刷新状态 |
| 激化领域 3.3 | 己方来源对对方出战角色造成雷或草伤害时，伤害增加 1，并消耗一层 | `count = 3`；重复生成按本次请求刷新状态 |
| 激化领域 3.4 | 增伤条件与 3.3 相同 | `count = 2`；重复生成按本次请求刷新状态 |
| 燃烧烈焰 3.3 | 回合结束时，对对方出战角色造成 `value` 点火伤害，随后消耗一次可用次数 | `value = 1`、`usages = 2`；重复召唤累加本次请求的可用次数，最多两次 |
| 冻结 3.3 | 角色受控；该角色受到物理或火伤害时加伤 2 并解除冻结；回合开始通知时解除 | `count = 1`、`round_usages = 0`；同定义重复施加保留已有实体 |

三类效果的伤害元素、增幅和使用次数见[游戏文本资料](https://gensh.honeyhunterworld.com/i_n333013/?lang=EN)。草原核和激化领域只增强对方出战角色受到的伤害，不增强对后台角色或己方角色的伤害；来源可以是己方角色、技能、召唤物等。

出战状态通过 [combat_status_state_limit](definition/queries/combat_status_state_limit.md) 提供表中的层数上限，其 `round_usages` 上限为零。每次增伤后通过状态修改命令扣层，层数归零时响应自身的状态变化并移除。重复生成使用本次已经裁剪的状态，按完整状态覆盖，不累加层数。

燃烧烈焰通过 [summon_state_limit](definition/queries/summon_state_limit.md) 限制效果量和次数。默认燃烧反应请求的状态是 `{ .value = 1, .usages = 1 }`，已有燃烧烈焰时累加一次。直接使用 [summon](definition/commands/summon.md) 或 [add_summon](definition/commands/add_summon.md) 且省略 `state`，仍遵守这些命令的规则，采用上限状态 `{ .value = 1, .usages = 2 }`。

燃烧烈焰的回合结束伤害完整结算后才扣除次数；燃烧烈焰具有 `remove_at_zero_usages` 标签，扣次的 [modify_summon_state](definition/commands/modify_summon_state.md) 在次数耗尽后将其移除并发送 [summon_removed](definition/events/summon_removed.md)。草原核和激化领域的移除发送 [combat_status_removed](definition/events/combat_status_removed.md)。

## 冻结与控制

`frozen_3_3_0_source` 的静态对象为 `frozen_3_3_0`。该附属具有 `control` 标签，供 [`is_controlled`](executor/definition_library/is_controlled.md) 判断角色是否受控；技能和主动特技的选择方应据此限制使用，执行器不自动检查技能提交。

默认冻结反应使本段伤害增加 1，并在本段扣血、击倒及附着处理后，向仍存活的反应目标施加冻结。新的冻结不追溯影响产生它的伤害，可影响后续同组伤害。替代反应会取消默认加伤与冻结生成；目标具有 `control_immunity` 时，只阻止控制附属的施加，不撤销已发生的反应或默认加伤。

冻结在 [`damage_calculation`](definition/events/damage_calculation.md) 中检查最终伤害类型：自己的角色受到物理或火伤害时，伤害饱和增加 2，并立即通过 [`remove_attachment`](definition/commands/remove_attachment.md) 移除自身，再继续本次数值广播和后续倍率、护盾处理。因此即使最终被护盾抵挡，也已解除冻结。其他类型不触发这一效果；移除冻结不移除角色身上的其他控制附属。

未被上述伤害解除的冻结响应 [`round_started`](definition/events/round_started.md) 离场，因此结束阶段仍保留冻结，期间新产生的冻结也持续到后续回合完成投骰与全部重投后的回合开始通知。冻结移除同样发送 [`attachment_removed`](definition/events/attachment_removed.md)。

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
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
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
