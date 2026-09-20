[givm](../../reference.md) / [牌桌](../table.md) / **character_state**

# givm::character_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct character_state;
```

角色在对局中的生命值、充能、元素附着和可装备的武器类别。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `max_health` | `std::uint32_t` | 生命值上限 |
| `max_energy` | `std::uint32_t` | 充能上限 |
| `health` | `std::uint32_t` | 当前生命值 |
| `energy` | `std::uint32_t` | 当前充能 |
| `aura` | [`element_aura`](../enums/element_aura.md) | 当前元素附着，默认 none |
| `allowed_weapon_types` | [`weapon_type_mask`](../enums/weapon_type_mask.md) | 允许装备的武器类别，默认全部为 false，表示不能装备武器 |
| `energy_tag` | [`tag_id`](tag_id.md) | 充能类型；默认无效 ID 表示普通充能，有效 ID 表示对应标签的替代充能 |

## 注意

普通充能和替代充能共用 `energy` 与 `max_energy`，分别表示当前点数与上限。角色在同一时刻只持有 `energy_tag` 指定的一种充能。

通过 [`operator[]`](../enums/weapon_type_mask/operator_at.md) 读取指定武器类别是否允许装备，通过 [`set`](../enums/weapon_type_mask/set.md) 和 [`reset`](../enums/weapon_type_mask/reset.md) 修改。例如，`state.allowed_weapon_types.set(weapon_type::sword)` 允许单手剑，`state.allowed_weapon_types.reset(weapon_type::sword)` 禁止单手剑。可以同时允许多个类别；`weapon_type::none` 不表示一种可装备的武器，不能传给这些接口。

这个状态记录当前装备限制。角色定义的武器类别标签仍可用于描述角色和判断其他效果；两者不自动同步。

要装备的武器类型由其 attachment 定义的 `sword`、`claymore`、`polearm`、`bow` 或 `catalyst` 标签决定，这些类型标签互斥。目标验证可以据此检查对应类别是否允许；添加命令不自动执行这项检查。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::character_state value{ .max_health = 10, .max_energy = 3, .health = 8, .energy = 1 };
    std::println("生命值: {}/{}", value.health, value.max_health);
    std::println("充能: {}/{}", value.energy, value.max_energy);
}
```

输出

```text
生命值: 8/10
充能: 1/3
```
