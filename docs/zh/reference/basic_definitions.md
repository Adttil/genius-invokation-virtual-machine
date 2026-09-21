[givm](../reference.md) / **基础定义源**

# 基础定义源

定义于头文件 `<givm/basic_definitions.hpp>`，也可通过 `<givm/givm.hpp>` 使用。

`givm::genshin_impact` 提供基础定义源的原神官方版本，供调用方明确选择默认元素反应关联的实体规则。当前提供以下占位源，可用于[构造定义源库](definition/definition_source_library/constructor.md)和编译；它们尚未实现实体的层数及事件响应效果。

## 定义源对象

| 对象 | 定义名称 | 类别 |
| --- | --- | --- |
| `dendro_core_3_3_0` | `dendro_core-3.3.0-genshin_impact` | `combat_status_view` |
| `catalyzing_field_3_3_0` | `catalyzing_field-3.3.0-genshin_impact` | `combat_status_view` |
| `catalyzing_field_3_4_0` | `catalyzing_field-3.4.0-genshin_impact` | `combat_status_view` |
| `burning_flame_3_3_0` | `burning_flame-3.3.0-genshin_impact` | `summon_view` |

这些对象具有静态生命周期，可直接传入源库。调用方可以选择激化领域的不同版本，也可以用自己的定义源替代任何一项。源库按构造参数确定其用途，不从上述名称推断用途。

版本后缀分别对应七圣召唤的 [3.3 版本推出](https://genshin.hoyoverse.com/en/news/detail/104486)及激化领域的 [3.4 版本调整](https://www.hoyolab.com/article/15092878)，表示计划实现的规则版本。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0
    };
    const auto [library, ids] = compile(
        sources, std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal);
    std::println("激化领域定义: {}", library.name(library.catalyzing_field_id()));
}
```

输出

```text
激化领域定义: catalyzing_field-3.4.0-genshin_impact
```
