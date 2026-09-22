[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **dendro_core_id**

# givm::definition_library::dendro_core_id

定义于头文件 `<givm/executor.hpp>`

```cpp
definition_id<combat_status_view> dendro_core_id() const noexcept;
```

取得默认元素反应所采用的草原核定义。该定义由[源库构造时](../../definition/definition_source_library/constructor.md)传入的源决定，可以是随库提供的版本，也可以是自定义版本。

## 返回值

本定义库中的有效 `definition_id<combat_status_view>`。即使编译时只选择了部分定义，该定义也会保留。

## 注意

返回的是定义 ID，不是已经存在于牌桌上的实体 ID。调用本函数不会创建实体。

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
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    const auto [library, ids] = compile(
        sources, givm::definition_selection{}, std::tuple{},
        std::tuple{ givm::start_round{} }, givm::compile_mode::normal);
    const auto id = library.dendro_core_id();
    std::println("采用指定的草原核源: {}", id == ids.get_id<givm::combat_status_view>(
        givm::genshin_impact::dendro_core_3_3_0.name()));
}
```

输出

```text
采用指定的草原核源: true
```
