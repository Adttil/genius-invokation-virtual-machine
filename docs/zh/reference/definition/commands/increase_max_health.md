[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **increase_max_health**

# givm::increase_max_health

定义于头文件 `<givm/definition.hpp>`

```cpp
struct increase_max_health;
```

增加一个角色的生命上限，并恢复相同数量的生命。完成后只发送 [`healed`](../events/healed.md) 通知，不经过 [`healing`](../events/healing.md) 治疗量修饰，也不产生额外的生命上限变更事件。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`relative_character_target`](../events/relative_character_target.md) | 固定来源角色的位置；默认本方的出战角色 |
| `target` | [`relative_character_target`](../events/relative_character_target.md) | 固定目标位置；默认采用动态输入 |
| `value` | `std::uint32_t` | 固定增加量；默认 `0` |

## 输入

- 默认构造 `increase_max_health{}`，消费响应通过 `invoke` 提交的一个 [`healing`](../events/healing.md)，读取其中的来源、目标和增加量；该输入不作为 `healing` 广播。
- 显式指定 `target` 时使用命令中的固定参数，不消费响应输入。

动态输入须指定有效角色。固定参数在命令执行时分别定位来源和目标；缺少任一角色时跳过命令。来源不会被替换成目标，也不会从外层响应推断；需要精确技能、牌或召唤物来源时应采用动态输入。

生命上限最多增加至 `std::uint32_t` 的最大值，治疗值为实际增加量。通知时上限和生命均已更新；增加量为 `0` 时仍通知。

例如角色生命为 `7/10`，增加 `3` 点生命上限后变为 `10/13`，完成通知中的 `value` 为 `3`。原本满血的角色也恢复相同数量，仍然满血。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const givm::relative_character_target target{ givm::relative_player::self, 0 };
    const givm::increase_max_health command{ .source = target, .target = target, .value = 3 };
    std::println("生命上限增加量: {}", command.value);
}
```

输出

```text
生命上限增加量: 3
```
