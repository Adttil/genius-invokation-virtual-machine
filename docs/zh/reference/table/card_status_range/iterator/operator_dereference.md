[givm](../../../../reference.md) / [牌桌](../../../table.md) / [card_status_range](../../card_status_range.md) / [iterator](../iterator.md) / **operator***

# givm::card_status_range::iterator::operator*

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr TStatusEntity operator*() const;
```

取得遍历当前位置的卡牌状态。

## 返回值

当前状态的 `TStatusEntity` 访问对象。

## 注意

当前位置不得是 `end()`。

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
    const example_source<givm::card_definition> card_source{};
    const example_source<givm::status_definition> status_source{};
    sources.add(card_source, status_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::card_definition>("示例");
    const auto entity = player.add_hand_card(definition, {});
    const auto status_definition = id_map.get_id<givm::status_definition>("示例");
    entity.add(status_definition, { .count = 2 });
    entity.add(status_definition, { .count = 3 });
    const auto statuses = entity.statuses();
    const auto status = *statuses.begin();
    std::println("当前状态计数: {}", status.state().count);
}
```

输出

```text
当前状态计数: 2
```
