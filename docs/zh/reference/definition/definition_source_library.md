[givm](../../reference.md) / [定义](../definition.md) / **definition_source_library**

# givm::definition_source_library

定义于头文件 `<givm/source_library.hpp>`

```cpp
class definition_source_library;
```

卡牌、角色和其他实体的定义源集合，供对局选择所需内容。它把分散编写的定义组织在一起，提供按类别和名称访问定义源的能力。默认元素反应采用哪些基础定义由编译时的 [`basic_definition_sources`](basic_definition_sources.md) 单独指定。

## 成员常量

|  |  |
| --- | --- |
| [`definition_count`](definition_source_library/definition_count.md) | 支持的定义类别数量 |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](definition_source_library/constructor.md) | 建立空源库 |
| [`add`](definition_source_library/add.md) | 登记定义源或合并源库 |
| [`has`](definition_source_library/has.md) | 检查定义源是否存在 |
| [`get`](definition_source_library/get.md) | 按名称查看定义源 |
| [`source_views`](definition_source_library/source_views.md) | 遍历指定类别的全部定义源 |

## 非成员函数

|  |  |
| --- | --- |
| [`make_definition_source_library`](make_definition_source_library.md) | 创建源库并批量登记定义源 |
| [`compile`](../executor/compile.md) | 编译选定定义与对局流程 |

## 注意

`<givm/source_library.hpp>` 提供完整类型，可以构造、析构、复制、移动和合并源库，也保留登记具体定义源的模板接口。内容库的公共头可直接包含它并声明返回本类型的函数；调用方链接内容库及 GIVM 后即可使用返回值。原有 `<givm/definition.hpp>` 仍可使用。

源库不拥有定义源。登记的源对象及名称、标签、依赖名称的字符存储必须在源库使用期间保持有效；编译出的定义库仍会使用名称和标签的字符存储。

登记后，源的名称、标签和依赖声明必须保持不变。登记、遍历和编译可以分别读取这些信息；每次返回的范围只消费一次，多次调用仍须提供相同内容。

默认构造得到空集合；通过 [`add`](definition_source_library/add.md) 登记源，结果中的错误列表提供名称冲突和缺失依赖的结构化诊断。同类别同名项若引用同一对象且类型相同则去重；其他错误会使整次登记或合并失败，并保留原有集合。

需要创建库并登记一批源时，使用 [`make_definition_source_library`](make_definition_source_library.md) 取得包含源库或诊断列表的结果。错误列表可以通过 [`error_string`](error_string.md) 转换为文本，源库不提供带源参数的构造函数。

## 示例

```cpp
#include <utility>
#include <print>
#include <string_view>
#include <tuple>

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
    const givm::basic_definition_sources basics{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    givm::definition_source_library sources{};
    std::println("登记成功: {}", sources.add(potion).has_value());
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
    std::println("可用卡牌: {}", library.name(ids.get_id<givm::card_definition>("恢复药剂")));
}
```

输出

```text
登记成功: true
可用卡牌: 恢复药剂
```

## 参阅

|  |  |
| --- | --- |
| [定义源协议](source_protocol.md) | 卡牌与角色定义源的编写协议 |
| [`definition_selection`](../executor/definition_selection.md) | 按类别指定的定义名称集合 |
