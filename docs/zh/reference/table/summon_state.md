[givm](../../reference.md) / [牌桌](../table.md) / **summon_state**

# givm::summon_state

定义于头文件 `<givm/table.hpp>`

召唤物的效果量与剩余可用次数。效果量的具体含义由定义决定，例如每次造成的伤害；可用次数耗尽时，执行器的状态修改命令会使召唤物离场。

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
| `usages` | `std::uint32_t` | 剩余可用次数；生成和添加新实体时，按定义上限裁剪后必须大于零 |

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
