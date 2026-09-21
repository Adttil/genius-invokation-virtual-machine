[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **(构造函数)**

# givm::definition_source_library::definition_source_library

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDendroCore, class TCatalyzingField, class TBurningFlame, class... TOtherSources>
definition_source_library(const TDendroCore& dendro_core,
                          const TCatalyzingField& catalyzing_field,
                          const TBurningFlame& burning_flame,
                          const TOtherSources&... other_sources);
```

指定本场规则中草原核、激化领域和燃烧烈焰采用的定义，并登记这些源及同时传入的其他源。三个反应定义必传，可选用随库提供的[基础定义源](../../basic_definitions.md)，也可以提供自定义版本。

## 模板参数

|  |  |
| --- | --- |
| `TDendroCore`、`TCatalyzingField` | `definition_category` 为 `combat_status_view` 的定义源类型 |
| `TBurningFlame` | `definition_category` 为 `summon_view` 的定义源类型 |
| `TOtherSources...` | 符合[定义源协议](../source_protocol.md)的其他源类型 |

## 参数

|  |  |
| --- | --- |
| `dendro_core` | 草原核采用的源 |
| `catalyzing_field` | 激化领域采用的源 |
| `burning_flame` | 燃烧烈焰采用的源 |
| `other_sources...` | 一起登记的其他源，可用于满足前三者的依赖 |

## 返回值

（无）

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 本批源有同类别名称冲突，或按名称声明的依赖缺失 |

## 注意

本函数不拥有源对象，遵守与 [`add`](add.md) 相同的生命周期约定。一批源可相互依赖，登记后仍可继续用 `add` 增补其他内容。

三个默认反应定义由参数位置决定，不要求源使用特定名称或版本。它们及其依赖在部分选择时也始终保留；编译后可通过定义库的 [`dendro_core_id`](../../executor/definition_library/dendro_core_id.md)、[`catalyzing_field_id`](../../executor/definition_library/catalyzing_field_id.md)、[`burning_flame_id`](../../executor/definition_library/burning_flame_id.md) 取得对应 ID。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    std::println("已登记恢复药剂: {}", sources.has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
已登记恢复药剂: false
```
