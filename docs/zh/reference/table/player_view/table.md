[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **table**

# givm::player_view::table

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const givm::table& table() const noexcept;
```

取得这个实体所属的牌桌，以查看同一对局中的其他实体和当前局面。

## 返回值

所属 [`table`](../table.md) 的只读引用。

## 注意

视图和返回的引用都不拥有牌桌。存活要求见[实体的身份与访问](../entity_access.md)。

## 示例

```cpp
#include <print>

#include <givm/table.hpp>

int main()
{
    givm::table table{ { .round_number = 4 } };
    const auto player = table[givm::player_id{ 0 }];
    std::println("当前回合: {}", player.table().state().round_number);
}
```

输出

```text
当前回合: 4
```
