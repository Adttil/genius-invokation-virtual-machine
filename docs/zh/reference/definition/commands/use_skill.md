[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **use_skill**

# givm::use_skill

定义于头文件 `<givm/definition.hpp>`

使出战角色立即使用一次技能，完整处理技能效果及使用后的通知。用于天赋牌等效果，不计算或支付技能本身的费用。

```cpp
struct use_skill
{
    relative_player player = relative_player::self;
    definition_id<skill_view> definition{};
};
```

## 输入

- 默认构造 `use_skill{}` 使用动态模式，由响应通过 `invoke` 提交一个 [`skill_effect`](../events/skill_effect.md)，明确指定技能实体、技能性质及目标。
- 显式指定 `definition` 时使用固定模式，不消费响应输入。在命令执行时，使用 `player` 指定一方的当前出战角色上首个匹配定义的技能。`player` 默认选择本方，沿用 [`relative_player`](relative_player.md) 的含义。

固定模式下，若该方尚未指定出战角色，或其出战角色没有匹配定义的技能，本命令无效果，也不发送技能使用前后的通知。命令不会创建技能或借用后台角色的技能。

固定模式从 [`definition_library::skill_flags`](../../executor/definition_library/skill_flags.md) 读取技能的固有分类，两个目标位置为空；需要传入技能目标时使用动态模式。动态模式保留输入的 `flags`，可通过同一查询取得固有分类。命令不会自动根据骰子数量或切换记录添加重击、下落攻击性质。

动态输入必须指定实际存在、且属于该方当前出战角色的技能实体。无论哪种模式，实际使用的技能都须支持 `skill_effect`，其所属角色须存活并未被控制；需要目标时须提供合法目标。命令不进行费用计算、支付或目标合法性查询，这些使用前提由定义源保证。

## 结算

1. 广播 [`skill_will_be_used`](../events/skill_will_be_used.md)，完成其全部响应程序。
2. 若技能效果未被取消，仅向该技能发送 [`skill_effect`](../events/skill_effect.md)，完整执行其效果程序。
3. 广播 [`skill_used`](../events/skill_used.md)，完成其全部响应程序后结束命令。技能效果被取消时仍发送此通知。

因此，天赋牌的 `card_effect` 程序包含本命令时，技能效果和所有 `skill_used` 响应会先完成；牌的其余效果继续执行，全部结束后才广播 [`card_played`](../events/card_played.md)。

技能使用不会自动增加充能。需要获得充能的技能应在自身效果程序中显式安排 [`modify_energy`](modify_energy.md)，其执行位置由程序中的命令顺序决定。

本命令的 `skill_will_be_used::speed` 初始为 `action_speed::fast`，响应可修改它，`skill_used` 保留修改后的值。该值不会改变外层行动的速度、当前行动玩家或下落攻击机会。天赋牌是否为战斗行动仍由牌的费用及出牌流程决定；本命令不读取技能费用。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/definition.hpp>

int main()
{
    const givm::any_command command{ givm::use_skill{} };
    const auto& skill_command = std::get<givm::use_skill>(command);
    std::println("默认选择本方: {}", skill_command.player == givm::relative_player::self);
    std::println("使用动态输入: {}", not skill_command.definition);
}
```

输出

```text
默认选择本方: true
使用动态输入: true
```
