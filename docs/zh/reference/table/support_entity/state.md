[givm](../../../reference.md) / [牌桌](../../table.md) / [support_entity](../support_entity.md) / **state**

# givm::support_entity::state

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr auto& state() const;
```

访问该支援在对局中的当前状态。

## 返回值

实体持有的 [`support_state`](../support_state.md) 引用；可写实体返回可写引用，只读视图返回只读引用。

## 注意

实体必须有效。直接修改状态不执行规则结算或事件广播。

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
    const example_source<givm::support_view> support_source{};
    sources.add(support_source);
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    const auto player = table[givm::player_id{ 0 }];
    const auto definition = id_map.get_id<givm::support_view>("示例");
    const auto entity = player.add(definition, { .count = 3 });
    entity.state().count = 2;
    std::println("计数: {}", entity.state().count);
}
```

输出

```text
计数: 2
```
