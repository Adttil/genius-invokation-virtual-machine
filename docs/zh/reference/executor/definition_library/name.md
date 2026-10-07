[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **name**

# givm::definition_library::name

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<definition_category TDefinitionType>
std::string_view name(definition_id<TDefinitionType> id) const;
```

取得实体定义的名称。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别 |

## 参数

|  |  |
| --- | --- |
| `id` | 本定义库中的有效定义 ID |

## 返回值

对应的定义名称。

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
    static constexpr auto category = givm::definition_category::card;

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const auto basics = givm::genshin_impact::reaction_names_3_3_0;
    givm::definition_source_library sources{};
    sources.add(givm::genshin_impact::reaction_sources_3_3_0());
    if(not sources.add(source)) return 1;
    auto library_result = compile(
        sources, basics,
        std::tuple{}, std::tuple{ givm::start_round{}, givm::settle{} }, givm::compile_mode::normal
    );
    if(not library_result)
    {
        std::println("{}", error_string(library_result.error()));
        return 1;
    }
    const auto [library, ids] = std::move(*library_result);
    const auto card = ids.get_id<givm::definition_category::card>("恢复药剂");
    std::println("卡牌名称: {}", library.name(card));
}
```

输出

```text
卡牌名称: 恢复药剂
```
