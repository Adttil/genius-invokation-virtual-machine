[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **find_tag**

# givm::definition_compile_context::find_tag

定义于头文件 `<givm/executor.hpp>`

```cpp
std::optional<tag_id> find_tag(std::string_view name) const;
```

查找本次编译集合中存在的标签。未被选入本场规则的定义不参与标签查询。

## 参数

|  |  |
| --- | --- |
| `name` | 要查找的标签名称 |

## 返回值

存在时返回配套定义库中的标签 ID；不存在时返回 `std::nullopt`。

## 注意

查询不需要依赖声明，也不会把未选择的定义加入编译集合。标签来自选中定义的 `tags()`；需要保证某个资源标签存在时，可以将其列为相关定义自身的标签。

## 示例

```cpp
#include <utility>
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "治疗检索"; }
    auto tags() const
    { return std::array<std::string_view, 1>{ "治疗" }; }

    auto compile(givm::definition_compile_context& context) const
    {
        const auto tag = context.find_tag("治疗");
        std::println("已取得治疗标签: {}", tag.has_value());
        std::println("存在未登记标签: {}", context.find_tag("未登记").has_value());
        return tag;
    }
};

int main()
{
    const card_source source{};
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
}
```

输出

```text
已取得治疗标签: true
存在未登记标签: false
```
