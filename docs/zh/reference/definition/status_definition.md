[givm](../../reference.md) / [定义](../definition.md) / **status_definition**

# givm::status_definition

定义于头文件 `<givm/definition.hpp>`

```cpp
struct status_definition {};
```

附着于卡牌的状态定义类别标记。同一状态可以随卡牌出现在手牌区或牌堆中；其定义与牌桌上某个状态实例的剩余次数等变化分开。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    const auto id = ids.add<givm::status_definition>("效果增强", {});
    std::println("已取得卡牌状态定义: {}", id.is_valid());
}
```

输出

```text
已取得卡牌状态定义: true
```

## 参阅

|  |  |
| --- | --- |
| [`views_of_definition`](views_of_definition.md) | 定义对应的实体形态 |
