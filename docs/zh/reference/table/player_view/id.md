[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **id**

# givm::player_view::id

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr player_id id() const;
```

取得玩家的身份，以便之后从牌桌再次访问它。

## 返回值

该实体的 [`player_id`](../player_id.md)。


## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{};
    const auto player = table[givm::player_id{ 1 }];
    std::println("玩家: {}", player.id().index);
}
```

输出

```text
玩家: 1
```
