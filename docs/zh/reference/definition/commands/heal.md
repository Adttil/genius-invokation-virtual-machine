[givm](../../../reference.md) / [定义](../../definition.md) / [命令](../commands.md) / **heal**

# givm::heal

定义于头文件 `<givm/definition.hpp>`

```cpp
struct heal;
```

使一个角色接受治疗。先广播 [`healing`](../events/healing.md) 供响应者调整治疗量，再恢复生命，最后广播 [`healed`](../events/healed.md) 通知实际恢复值。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `source` | [`effect_source_id`](../events/effect_source_id.md) | 固定治疗来源 |
| `target` | [`character_id`](../../table/character_id.md) | 固定目标；默认采用动态输入 |
| `value` | `std::uint32_t` | 固定治疗量；默认 `0` |

## 输入

- 默认构造 `heal{}`，消费响应通过 `invoke` 提交的一个 [`healing`](../events/healing.md)。
- 显式指定 `target` 时使用命令中的固定参数，不消费响应输入。

目标必须是有效角色。允许目标生命值为 `0`，因此濒死响应可以通过本命令恢复生命。

## 结算

`healing` 的所有响应和响应程序完成后，以此时的生命值和生命上限截断治疗量。恢复量为 `min(value, max_health - health)`，`healed::value` 记录这一实际值。

满血或治疗量为 `0` 时仍完整处理两次广播；完成通知中的实际值为 `0`。响应程序可以暂停等待输入，恢复后继续本次治疗。

## 示例

```cpp
#include <print>
#include <givm/definition.hpp>

int main()
{
    const givm::character_id target{ givm::player_id{ 0 }, 0 };
    const givm::heal command{ .source = target, .target = target, .value = 2 };
    std::println("治疗量: {}", command.value);
}
```

输出

```text
治疗量: 2
```
