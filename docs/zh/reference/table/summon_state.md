[givm](../../reference.md) / [牌桌](../table.md) / **summon_state**

# givm::summon_state

定义于头文件 `<givm/table.hpp>`

召唤物的效果量与剩余可用次数。效果量的具体含义由定义决定，例如每次造成的伤害；可用次数归零后是否离场，由该召唤物的[状态修改响应](../definition/events/summon_state_changed.md)决定。

```cpp
struct summon_state
{
    std::uint32_t value;
    std::uint32_t usages;
};
```

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `value` | `std::uint32_t` | 效果量 |
| `usages` | `std::uint32_t` | 剩余可用次数，允许为零 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::summon_state value{ .value = 1, .usages = 3 };
    --value.usages;
    std::println("剩余次数: {}", value.usages);
}
```

输出

```text
剩余次数: 2
```
