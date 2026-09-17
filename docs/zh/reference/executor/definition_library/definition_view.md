[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **definition_view**

# givm::definition_library::definition_view

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TDefinitionType>
class definition_view;
```

一项实体定义的只读视图。它描述实体采用的共同规则与分类，不随某个实体的生命值、可用次数或所在区域等对局状态一起变化。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别 |

## 成员函数

|  |  |
| --- | --- |
| [`id`](definition_view/id.md) | 取得定义 ID |
| [`name`](definition_view/name.md) | 取得定义名称 |
| [`has_tag`](definition_view/has_tag.md) | 检查一个标签 |
| [`has_all_tags`](definition_view/has_all_tags.md) | 检查全部标签 |
| [`has_any_tag`](definition_view/has_any_tag.md) | 检查任一标签 |
| [`matches_tags`](definition_view/matches_tags.md) | 检查标签筛选条件 |
| [`can_handle`](definition_view/can_handle.md) | 检查事件响应能力 |
| [`handle`](definition_view/handle.md) | 请求响应一个事件 |
| [`query`](definition_view/query.md) | 取得规则信息或检查结果 |

## 注意

由 [`definition_library::operator[]`](operator_at.md) 取得。查询实体定义时，显式使用 `library[entity.definition_id()]`。视图不拥有定义库，所引用的定义库应保持有效。

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
    const auto definition = library[card];
    std::println("卡牌名称: {}", definition.name());
    std::println("具有治疗标签: {}", definition.has_tag(ids.get_tag_id("治疗")));
}
```

输出

```text
卡牌名称: 恢复药剂
具有治疗标签: true
```
