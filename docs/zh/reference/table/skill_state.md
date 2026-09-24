[givm](../../reference.md) / [牌桌](../table.md) / **skill_state**

# givm::skill_state

定义于头文件 `<givm/table.hpp>`

```cpp
struct skill_state;
```

技能在对局中使用的计数状态。`count` 的具体含义由相应定义决定，例如剩余可用次数或累计数量。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 该实体当前的计数 |

## 注意

效果程序可通过 [`set_skill_state`](../definition/commands/set_skill_state.md) 设置完整状态。命令不限制或裁剪 `count`，也不因其为零而删除技能；计数的含义与更新时机由技能定义决定。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::skill_state value{ .count = 3 };
    --value.count;
    std::println("剩余计数: {}", value.count);
}
```

输出

```text
剩余计数: 2
```
