[givm](../../reference.md) / [执行](../executor.md) / **definition_library**

# givm::definition_library

定义于头文件 `<givm/executor.hpp>`

```cpp
class definition_library;
```

一场对局使用的卡牌、角色和其他实体的定义集合，以及这场对局应当遵循的流程。实体具有相同定义时，便具有相同的基础规则和事件响应能力；它们各自在对局中的变化由牌桌记录。

## 成员类型

|  |  |
| --- | --- |
| [`definition_view`](definition_library/definition_view.md) | 一项定义的只读视图 |

## 成员常量

|  |  |
| --- | --- |
| [`definition_count`](definition_library/definition_count.md) | 支持的定义类别数量 |

## 成员函数

|  |  |
| --- | --- |
| [`operator[]`](definition_library/operator_at.md) | 查看指定定义 |
| [`name`](definition_library/name.md) | 取得定义名称 |
| [`tag_name`](definition_library/tag_name.md) | 取得标签名称 |
| [`skill_flags`](definition_library/skill_flags.md) | 取得技能定义标签对应的固有分类 |
| [`equipment_type`](definition_library/equipment_type.md) | 取得附着实体定义的装备类别 |
| [`remove_at_zero_usages`](definition_library/remove_at_zero_usages.md) | 检查召唤物定义是否按零次数离场 |
| [`is_control`](definition_library/is_control.md) | 检查附属定义是否属于控制效果 |
| [`is_controlled`](definition_library/is_controlled.md) | 检查角色是否处于控制状态 |
| [`is_control_immune`](definition_library/is_control_immune.md) | 检查角色是否免疫控制附属与效果切人 |
| [`has_tag`](definition_library/has_tag.md) | 检查定义是否具有一个标签 |
| [`has_all_tags`](definition_library/has_all_tags.md) | 检查定义是否具有全部标签 |
| [`has_any_tag`](definition_library/has_any_tag.md) | 检查定义是否具有任一标签 |
| [`matches_tags`](definition_library/matches_tags.md) | 按必需与排除标签检查定义 |
| [`can_handle`](definition_library/can_handle.md) | 检查定义能否响应事件 |
| [`handle`](definition_library/handle.md) | 请求定义响应事件 |
| [`query`](definition_library/query.md) | 取得定义的规则信息或检查结果 |
| [`dendro_core_id`](definition_library/dendro_core_id.md) | 取得默认草原核定义 ID |
| [`catalyzing_field_id`](definition_library/catalyzing_field_id.md) | 取得默认激化领域定义 ID |
| [`burning_flame_id`](definition_library/burning_flame_id.md) | 取得默认燃烧烈焰定义 ID |
| [`frozen_id`](definition_library/frozen_id.md) | 取得默认冻结定义 ID |

## 注意

通过 [`compile`](compile.md) 创建。配套 ID 与程序入口应始终用于产生它们的定义库；定义视图的使用期不能超出定义库的生命期。牌桌不持有定义库，执行时由调用方显式传入配套的库。名称和标签的字符存储仍须保持有效。

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
    givm::definition_source_library sources{
        givm::genshin_impact::dendro_core_3_3_0,
        givm::genshin_impact::catalyzing_field_3_4_0,
        givm::genshin_impact::burning_flame_3_3_0,
        givm::genshin_impact::frozen_3_3_0
    };
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{} }, givm::compile_mode::normal
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    std::println("卡牌名称: {}", library.name(card));
    std::println("具有治疗标签: {}", library.has_tag(card, ids.get_tag_id("治疗")));
}
```

输出

```text
卡牌名称: 恢复药剂
具有治疗标签: true
```
