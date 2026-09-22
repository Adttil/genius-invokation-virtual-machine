[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **character_will_be_defeated**

# givm::character_will_be_defeated

定义于头文件 `<givm/definition.hpp>`

```cpp
struct character_will_be_defeated;
```

伤害实际扣血至零时的濒死通知，在终局判定和死亡清理之前全局广播。角色自身的技能和尚未清除的附属也能响应。

响应可提交 [`heal`](../commands/heal.md) 等恢复生命的程序。广播及全部响应程序完成后，执行器读取目标的实际生命：非零表示复活成功；仍为零则先判定是否终局，对局继续时才清除该角色全部附属和充能。事件没有独立的“阻止击倒”标记，也不自动恢复生命。

广播期间可以执行其他程序和等待输入，复制执行器及牌桌后也可独立恢复。观察模式先报告扣血现场，恢复执行后才进入本广播。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `target` | `const character_id` | 生命已降至零的角色；只读 |

## 示例

```cpp
#include <print>
#include <variant>

#include <givm/givm.hpp>

int main()
{
    givm::character_will_be_defeated event{ .target = { givm::player_id{ 1 }, 0 } };
    std::println("濒死角色位置: {}", event.target.index);
}
```

输出

```text
濒死角色位置: 0
```
