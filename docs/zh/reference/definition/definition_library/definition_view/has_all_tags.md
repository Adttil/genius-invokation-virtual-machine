[givm](../../../../reference.md) / [定义](../../../definition.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **has_all_tags**

# givm::definition_library::definition_view::has_all_tags

定义于头文件 `<givm/definition.hpp>`

```cpp
bool has_all_tags(std::span<const tag_id> tags) const;
```

检查该定义是否具有给定标签中的全部标签。

## 参数

|  |  |
| --- | --- |
| `tags` | 配套定义库中的有效标签 ID 范围 |

## 返回值

满足条件时返回 `true`，否则返回 `false`。空范围返回 `true`。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    const std::array tags{ ids.get_tag_id("治疗") };
    std::println("符合分类条件: {}", library[card].has_all_tags(tags));
}
```

输出

```text
符合分类条件: true
```
