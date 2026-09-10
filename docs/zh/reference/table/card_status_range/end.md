[givm](../../../reference.md) / [牌桌](../../table.md) / [card_status_range](../card_status_range.md) / **end**

# givm::card_status_range::end

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr iterator end() const noexcept;
```

取得卡牌状态序列的遍历终点。

## 返回值

不能解引用的尾后 [`iterator`](iterator.md)。

## 示例

```cpp
#include <cstddef>
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
    std::size_t count = 0;
    for(auto it = statuses.begin(); it != statuses.end(); ++it)
    {
        ++count;
    }
    std::println("状态数量: {}", count);
}
```

输出

```text
状态数量: 2
```
