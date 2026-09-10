[givm](../../reference.md) / [事件](../events.md) / **calculating_switch_payment**

# givm::calculating_switch_payment

定义于头文件 `<givm/executor/events.hpp>`

```cpp
struct calculating_switch_payment;
```

正在计算切换出战角色所需的支付内容。事件标识这次切换的目标角色。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 打算切换为出战角色的目标；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::calculating_switch_payment event{ .target = { .player_id = givm::player_id{ 0 }, .index = 1 } };
    std::println("目标角色序号: {}", event.target.index);
}
```

输出

```text
目标角色序号: 1
```
