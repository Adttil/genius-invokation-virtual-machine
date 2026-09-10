[givm](../../../reference.md) / [牌桌](../../table.md) / [player_entity](../player_entity.md) / **is_mutable**

# givm::player_entity::is_mutable

定义于头文件 `<givm/table.hpp>`

```cpp
static constexpr bool is_mutable = not std::is_const_v<TStorage>;
```

该访问对象是否允许修改玩家。可写实体为 `true`，只读视图为 `false`。

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
    std::println("实体允许修改: {}", decltype(entity)::is_mutable);
    std::println("视图允许修改: {}", decltype(view)::is_mutable);
}
```

输出

```text
实体允许修改: true
视图允许修改: false
```
