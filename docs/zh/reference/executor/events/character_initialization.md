[givm](../../../reference.md) / [执行](../../executor.md) / [事件](../events.md) / **character_initialization**

# givm::character_initialization

定义于头文件 `<givm/executor.hpp>`

```cpp
struct character_initialization;
```

角色初始状态的准备事件。角色定义通过它提供最大生命、初始生命、充能上限等开局状态。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `state` | [`character_state`](../../table/character_state.md) | 要赋给角色的初始状态，初始各成员按 character_state 的默认值初始化 |

## 注意

[`enter_character`](../instructions/enter_character.md) 和 [`initialize_characters`](../instructions/initialize_characters.md) 要求在此次 `handle` 中直接填写 `state`；它们不执行该响应返回的程序入口。

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::character_initialization event{};
    event.state.max_health = 10;
    event.state.health = 10;
    event.state.max_energy = 3;
    std::println("初始生命: {}/{}", event.state.health, event.state.max_health);
    std::println("充能上限: {}", event.state.max_energy);
}
```

输出

```text
初始生命: 10/10
充能上限: 3
```

## 参阅

| | |
| --- | --- |
| [`enter_character`](../instructions/enter_character.md) | 角色入场指令 |
| [`initialize_characters`](../instructions/initialize_characters.md) | 已有角色的初始化指令 |
