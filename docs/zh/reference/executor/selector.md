[givm](../../reference.md) / [执行](../executor.md) / **selector**

# givm::selector

定义于头文件 `<givm/executor.hpp>`

```cpp
struct selector;
```

一名玩家对一组候选项的选择结果。它记录哪些位置被选中，位置代表的对象由使用该选择的指令决定。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | [`player_id`](../table/player_id.md) | 进行选择的玩家 |
| `selected` | `std::bitset<selection_capacity>` | 各候选位置的选中状态 |

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::selector selection{ .player = givm::player_id{ 0 } };
    selection.selected.set(1);
    selection.selected.set(3);
    std::println("选择数量: {}", selection.selected.count());
    std::println("是否选择位置 1: {}", selection.selected.test(1));
}
```

输出

```text
选择数量: 2
是否选择位置 1: true
```

## 参阅

|  |  |
| --- | --- |
| [`selection_capacity`](selection_capacity.md) | 单次选择可表示的位置数量 |
