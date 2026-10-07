[givm](../../reference.md) / [牌桌](../table.md) / **definition_id**

# givm::definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
template<definition_category Category>
class definition_id;
```

一份卡牌、角色或持续效果规则的身份。不同类别使用不同的强类型；它标识实体采用的定义，不标识牌桌上的某个实体。

## 模板参数

`Category` 是实际的 [`definition_category`](../enums/definition_category.md)，不能是 `null`。

## 成员

| 名称 | 说明 |
| --- | --- |
| `category` | 静态定义类别 |
| [`(构造函数)`](definition_id/constructor.md) | 默认构造或从定义索引构造 |
| [`operator=`](definition_id/operator_assign.md) | 从整数索引赋值 |
| [`value`](definition_id/value.md) | 取得定义索引 |
| [`operator==`](definition_id/operator_equal.md) | 比较同类别定义 ID |

## 快捷别名

- [`card_definition_id`、`optional_card_definition_id`](card_definition_id.md)
- [`card_status_definition_id`、`optional_card_status_definition_id`](card_status_definition_id.md)
- [`support_definition_id`、`optional_support_definition_id`](support_definition_id.md)
- [`summon_definition_id`、`optional_summon_definition_id`](summon_definition_id.md)
- [`combat_status_definition_id`、`optional_combat_status_definition_id`](combat_status_definition_id.md)
- [`character_definition_id`、`optional_character_definition_id`](character_definition_id.md)
- [`skill_definition_id`、`optional_skill_definition_id`](skill_definition_id.md)
- [`attachment_definition_id`、`optional_attachment_definition_id`](attachment_definition_id.md)
- [`history_summary_definition_id`、`optional_history_summary_definition_id`](history_summary_definition_id.md)
- [`reaction_definition_id`、`optional_reaction_definition_id`](reaction_definition_id.md)

## 注意

该类型不表示空值，也没有有效性布尔判断。需要空值时使用 `optional_definition_id<Category>`；需要多个类别时使用 [`variant_definition_id`](variant_definition_id.md)。

默认构造保持平凡；未经初始化的 ID 只能作为赋值目标。整数构造不检查索引是否属于某份定义库。ID 不保存定义库身份；不同编译产物中的相同索引不保证代表相同规则。

ID 占一个 `std::uint64_t`。类别编码预留在高位，索引可用位数由末尾 `null` 推导，目前为 60 位。Debug 检查整数是否超出索引位宽，失败抛出 `std::invalid_argument`；Release 不执行该检查。

## 示例

```cpp
#include <givm/definition_source_interface.hpp>
#include <print>

int main()
{
    givm::issued_id_map ids{};
    const auto card = ids.add<givm::definition_category::card>("恢复药剂", {});
    givm::optional_card_definition_id chosen;
    std::println("初始没有定义: {}", not chosen);
    chosen = card;
    std::println("卡牌定义索引: {}", chosen.get().value());
}
```

输出

```text
初始没有定义: true
卡牌定义索引: 0
```
