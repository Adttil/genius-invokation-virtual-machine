[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **matches_tags**

# givm::definition_library::matches_tags

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TDefinitionType>
bool matches_tags(
    definition_id<TDefinitionType> id,
    std::span<const tag_id> required_tags,
    std::span<const tag_id> excluded_tags = {}
) const;
```

检查定义是否同时满足所需标签和排除标签的条件。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别 |

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |
| `required_tags` | 必须全部具有的标签 |
| `excluded_tags` | 必须全部不具有的标签 |

## 返回值

全部条件满足时返回 `true`，否则返回 `false`。两个标签范围均为空时返回 `true`。

## 注意

所有标签 ID 须属于本定义库且有效。

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
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::normal
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    const std::array healing{ ids.get_tag_id("治疗") };
    std::println("要求治疗标签: {}", library.matches_tags(card, healing));
    std::println("排除治疗标签: {}", library.matches_tags(card, {}, healing));
}
```

输出

```text
要求治疗标签: true
排除治疗标签: false
```
