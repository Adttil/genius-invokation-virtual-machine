[givm](../../reference.md) / [定义](../definition.md) / **definition_source_library**

# givm::definition_source_library

定义于头文件 `<givm/definition.hpp>`

```cpp
class definition_source_library;
```

卡牌、角色和其他实体的定义源集合，供对局选择所需内容。它把分散编写的定义组织在一起，提供按类别和名称访问定义源的能力，并确定元素反应采用哪些草原核、激化领域、燃烧烈焰和冻结定义。

## 成员常量

|  |  |
| --- | --- |
| [`definition_count`](definition_source_library/definition_count.md) | 支持的定义类别数量 |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](definition_source_library/constructor.md) | 指定并登记默认反应定义源 |
| [`add`](definition_source_library/add.md) | 登记定义源或合并源库 |
| [`has`](definition_source_library/has.md) | 检查定义源是否存在 |
| [`get`](definition_source_library/get.md) | 按名称查看定义源 |
| [`source_views`](definition_source_library/source_views.md) | 遍历指定类别的全部定义源 |
| [`make_issued_id_map`](definition_source_library/make_issued_id_map.md) | 为选定定义建立 ID 映射 |

## 非成员函数

|  |  |
| --- | --- |
| [`compile`](../executor/compile.md) | 编译选定定义与对局流程 |

## 注意

源库不拥有定义源。登记的源对象及名称、标签、依赖名称的字符存储必须在源库使用期间保持有效；编译出的定义库仍会使用名称和标签的字符存储。

登记后，源的名称、标签和依赖声明必须保持不变。登记、遍历和编译可以分别读取这些信息；每次返回的范围只消费一次，多次调用仍须提供相同内容。

## 示例

```cpp
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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    std::println("登记成功: {}", sources.add(potion));
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::normal
    );
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
| [`definition_selection`](definition_selection.md) | 按类别指定的定义名称集合 |
