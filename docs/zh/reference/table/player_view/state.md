[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **state**

# givm::player_view::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const player_state& state() const;
```

访问该玩家在对局中的当前状态。

## 返回值

实体持有的 [`player_state`](../player_state.md) 引用，只允许读取。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    const givm::player_view player = table[givm::player_id{ 0 }];
    std::println("初始骰子数: {}", player.state().dice.total());
}
```

输出

```text
初始骰子数: 0
```
