[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **operator[]**

# givm::definition_library::operator[]

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TDefinitionType>
definition_view<TDefinitionType> operator[](definition_id<TDefinitionType> id) const;
```

查看指定实体定义的名称、标签和事件响应能力。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别 |

## 参数

|  |  |
| --- | --- |
| `id` | 由本库配套映射发放的有效定义 ID |

## 返回值

该定义的只读 [`definition_view`](definition_view.md)。

## 注意

返回的视图不拥有定义库，定义库应保持有效。

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
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }
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
