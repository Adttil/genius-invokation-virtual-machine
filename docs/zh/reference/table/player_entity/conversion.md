[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **operator player_view**

# givm::player_entity::operator player_view

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr operator player_entity<const detail::table_storage>() const noexcept
    requires is_mutable;
```

将玩家的可写访问对象转换为同一实体的只读视图。

## 返回值

访问同一实体的 [`player_view`](../player_view.md)。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto entity = table[givm::player_id{ 0 }];
    const givm::player_view view = entity;
    std::println("骰子数: {}", view.state().dice.total());
}
```

输出

```text
骰子数: 0
```
