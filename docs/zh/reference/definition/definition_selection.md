[givm](../../reference.md) / [定义](../definition.md) / **definition_selection**

# givm::definition_selection

定义于头文件 `<givm/definition.hpp>`

```cpp
using definition_selection =
    std::array<std::span<const std::string_view>, definition_types::size()>;
```

一场对局首先需要的定义名称集合，例如双方牌组中出现的卡牌和角色。建立 [ID 映射](definition_source_library/make_issued_id_map.md)或[编译定义库](../executor/compile.md)时，会自动补入源库构造时选定的三个默认反应定义，以及这些定义和所选定义声明的依赖，无须调用方逐一列出。

## 注意

使用 `definition_types::index_of<T>()` 选择类别。每个范围包含该类别的定义名称；默认初始化的选择为空。范围和名称的字符存储须在选择、编译调用期间有效。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    const card_source food{ "恢复料理" };
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    sources.add(potion, food);
    const std::array<std::string_view, 1> names{ "恢复药剂" };
    givm::definition_selection selection{};
    selection[givm::definition_types::index_of<givm::card_definition>()] = names;
    const auto ids = sources.make_issued_id_map(selection);
    std::println("包含恢复药剂: {}", ids.has<givm::card_definition>("恢复药剂"));
    std::println("包含恢复料理: {}", ids.has<givm::card_definition>("恢复料理"));
}
```

输出

```text
包含恢复药剂: true
包含恢复料理: false
```
