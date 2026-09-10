[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **id**

# givm::player_entity::id

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
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto entity = table[givm::player_id{ 0 }];
    const auto id = entity.id();
    std::println("再次找到同一实体: {}", table[id].id() == id);
}
```

输出

```text
再次找到同一实体: true
```
