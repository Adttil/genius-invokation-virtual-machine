[givm](../../../reference.md) / [牌桌](../../table.md) / [summon_entity](../summon_entity.md) / **player**

# givm::summon_entity::player

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr player_type player() const;
```

取得这个召唤物所属的玩家。

## 返回值

具有相同读写权限的 [`player_entity`](../player_entity.md)。

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

template<class Category>
struct example_source
{
    using definition_category = Category;
    struct definition_type {};

    std::string_view name() const { return "示例"; }
    definition_type compile(givm::definition_compile_context&) const { return {}; }
};

int main()
{
    givm::definition_source_library sources{};
    const example_source<givm::summon_view> summon_source{};
    sources.add(summon_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{};
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::summon_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
    std::println("所属玩家: {}", entity.player().id().index);
}
```

输出

```text
所属玩家: 0
```
