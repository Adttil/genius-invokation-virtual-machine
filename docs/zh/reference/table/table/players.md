[givm](../../../reference.md) / [牌桌](../../table.md) / [table](../table.md) / **players**

# givm::table::players

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto players() const;
```

遍历参加这场对局的双方玩家。

## 返回值

依次产生玩家 0 和玩家 1 的 [`player_view`](../player_view.md) 的范围。

## 注意

范围及其中的实体访问对象依赖牌桌的存活。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    for(const auto player : table.players())
    {
        std::println("玩家 {} 的骰子数: {}", player.id().index, player.state().dice.total());
    }
}
```

输出

```text
玩家 0 的骰子数: 0
玩家 1 的骰子数: 0
```
