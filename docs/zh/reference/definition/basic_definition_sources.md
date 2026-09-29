[givm](../../reference.md) / [定义](../definition.md) / **basic_definition_sources**

# givm::basic_definition_sources

定义于头文件 `<givm/definition_source_interface.hpp>`

```cpp
struct basic_definition_sources;
```

一场对局的默认元素反应所采用的基础定义源配置。通过成员名称确定用途，定义源自身的名称和版本不受限制。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `dendro_core` | `definition_source_view<combat_status_view>` | 绽放生成的草原核定义源 |
| `catalyzing_field` | `definition_source_view<combat_status_view>` | 激化生成的激化领域定义源 |
| `burning_flame` | `definition_source_view<summon_view>` | 燃烧召唤的燃烧烈焰定义源 |
| `frozen` | `definition_source_view<attachment_view>` | 冻结附属的定义源 |
| `shield` | `definition_source_view<combat_status_view>` | 结晶生成的护盾定义源 |

## 注意

五个成员均须指定，可使用随库提供的[基础定义源](../basic_definitions.md)，也可使用相应类别的自定义源。各成员通过 [`definition_source_view`](definition_source_view.md) 借用源对象，不复制或拥有源对象。

[`compile`](../executor/compile.md) 显式接收本配置。即使只选择部分普通定义，五个基础定义及其依赖也始终保留。基础源不必预先登记到源库，编译也不会将它们写回源库。它们声明的其他依赖须由源库或本配置中的其他源满足。

定义源编译时可通过 [`definition_compile_context`](../executor/definition_compile_context.md) 的 `dendro_core_id()`、`catalyzing_field_id()`、`burning_flame_id()`、`frozen_id()`、`shield_id()` 取得本次配置对应的 ID，无须声明或查询具体版本名称。编译后的 [`definition_library`](../executor/definition_library.md) 提供同名查询。

本配置借用的源须覆盖映射准备及编译过程；名称、标签和编译数据借用对象的生命周期继续遵循[定义源协议](source_protocol.md)。预先发放 ID 和随后编译必须使用相同配置，不能跨不同配置混用 ID。
