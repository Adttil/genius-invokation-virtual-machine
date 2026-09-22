[givm](../../reference.md) / [牌桌](../table.md) / **support_state**

# givm::support_state

定义于头文件 `<givm/table.hpp>`

支援的层数与本回合剩余次数。两者独立保存；归零是否离场由该状态自己的响应决定。

```cpp
struct support_state
{
    std::uint32_t count;
    std::uint32_t round_usages;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `count` | `std::uint32_t` | 当前层数或定义自行解释的计数 |
| `round_usages` | `std::uint32_t` | 本回合剩余可用次数 |

每回合次数的重置由定义在相应回合事件中，通过 [set_support_state](../definition/commands/set_support_state.md) 表达；次数上限由 [support_state_limit](../definition/queries/support_state_limit.md) 给出，重置时机由定义决定。

## 自定义计数编码

`count` 的含义由定义决定，也可保存少量编码数据。例如需要保存最多两枚骰子的种类时，可用低两位记录数量，再用两个三位字段记录种类；显示时只取 `count & 3`。定义须自行保证编码合法。

这类定义应让 [`support_state_limit`](../definition/queries/support_state_limit.md) 的 `count` 上限覆盖完整编码范围，例如 `0xff`，并在创建时显式给出 `.state = { .count = 0 }`。修改编码使用 [`set_support_state`](../definition/commands/set_support_state.md) 写入完整新值；普通层数的饱和加减不适用于整个编码值。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::support_state value{ .count = 3 };
    --value.count;
    std::println("剩余计数: {}", value.count);
}
```

输出

```text
剩余计数: 2
```
