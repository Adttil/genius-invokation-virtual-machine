[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **source_views**

# givm::definition_source_library::source_views

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinitionType>
auto source_views() const;
```

遍历指定类别中全部已登记的定义源，读取其名称、标签和依赖声明。

## 模板参数

|  |  |
| --- | --- |
| `TDefinitionType` | 定义类别，见 [`definition_types`](../definition_types.md) |

## 返回值

由 [`definition_source_view<TDefinitionType>`](../definition_source_view.md) 构成的只读范围。没有登记该类别的源时返回空范围。

## 注意

范围借用源库，遍历期间不修改源库。单独取得的 source view 仅借用源对象，遵循 [`definition_source_view`](../definition_source_view.md) 的生命周期约定。遍历顺序不表示编译所得定义 ID 的顺序。

## 示例

```cpp
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "恢复药剂"; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{};
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(potion);
    for(const auto source : sources.source_views<givm::card_definition>())
    {
        std::println("卡牌定义源: {}", source.name());
    }
}
```

输出

```text
卡牌定义源: 恢复药剂
```

## 参阅

|  |  |
| --- | --- |
| [`get`](get.md) | 按名称查看定义源 |
| [`make_issued_id_map`](make_issued_id_map.md) | 为选定定义建立 ID 映射 |
