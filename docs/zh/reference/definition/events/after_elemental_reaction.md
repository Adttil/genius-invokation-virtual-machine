[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **after_elemental_reaction**

# givm::after_elemental_reaction

反应事实的只读通知，在当前段混合通知中按创建顺序处理。含 `source`、`target`、`incoming_element`、`reacted_aura`、`reaction_id reaction` 和 `cause`。取消默认后续效果不会删除本事件。

`source_player()` 返回引发玩家，`reaction.slot` 返回元素组合槽位。该槽位的实际定义可以按玩家映射替换；多种扩散和结晶分别保留自己的槽位。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::after_elemental_reaction event{ .source = givm::character_id{}, .target = {}, .incoming_element = givm::element::pyro, .reacted_aura = givm::element_aura::hydro, .reaction = { givm::player_id{ 0 }, givm::elemental_reaction::vaporize } };
    std::println("发生蒸发: {}", event.reaction.slot() == givm::elemental_reaction::vaporize);
    std::println("由独立效果附着: {}", event.cause == givm::element_application_cause::effect);
}
```

输出

```text
发生蒸发: true
由独立效果附着: true
```

## 参阅

| | |
| --- | --- |
| [`apply_element`](../commands/apply_element.md) | 元素附着命令 |
