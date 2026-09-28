[givm](../../reference.md) / [执行](../executor.md) / **command_input_error_reason**

# givm::command_input_error_reason

定义于头文件 `<givm/executor.hpp>`

```cpp
using command_input_error_reason = std::variant<invalid_entity_argument,
    invalid_definition_argument, invalid_enum_argument, duplicate_entity_argument,
    missing_entity_argument, invalid_numeric_argument, invalid_entity_relation,
    insufficient_dice_argument>;
```

[`command_input_error`](command_input_error.md) 的具体原因。以下类型均位于 `givm` 命名空间。

## 类

### `invalid_entity_argument`

```cpp
struct invalid_entity_argument;
```

实体参数越界或已经离场。嵌套枚举 `reason` 包含 `out_of_range` 和 `removed`。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 出错字段或所属实体字段的路径 |
| `entity` | `command_entity_id` | 保留具体 ID 类型的出错实体 |
| `cause` | `reason` | 索引越界或实体已经离场 |

### `invalid_definition_argument`

```cpp
struct invalid_definition_argument;
```

定义 ID 的数值不在所用定义库对应类别的范围内。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 出错字段路径 |
| `category_index` | `std::size_t` | 定义类别在 `definition_types` 中的索引 |
| `value` | `std::size_t` | 输入定义 ID 的数值 |
| `count` | `std::size_t` | 此类别的定义数量，即有效 ID 上界（不含） |

### `invalid_enum_argument`

```cpp
struct invalid_enum_argument;
```

枚举参数不在该操作允许的取值中。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 出错字段路径 |
| `value` | `std::size_t` | 枚举的数值 |

### `duplicate_entity_argument`

```cpp
struct duplicate_entity_argument;
```

不允许重复的实体列表中出现了重复项。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 列表字段路径 |
| `first_index` | `std::size_t` | 相同实体首次出现的位置，从零开始 |
| `index` | `std::size_t` | 重复项的位置，从零开始 |

### `missing_entity_argument`

```cpp
struct missing_entity_argument;
```

按定义或装备类型定位时没有找到操作要求存在的实体。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 目标字段路径 |
| `owner` | `std::optional<command_entity_id>` | 可确定时的所属实体 |
| `definition` | `std::optional<std::size_t>` | 按定义定位时使用的定义 ID 数值 |
| `equipment` | `std::optional<equipment_type>` | 按装备类型定位时使用的类别 |

### `invalid_numeric_argument`

```cpp
struct invalid_numeric_argument;
```

数值不符合该字段的约束。嵌套枚举 `constraint_kind` 包含 `at_most`（不大于上限）、`less_than`（严格小于上限）和 `nonzero`（非零）。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 出错字段路径 |
| `value` | `std::uint64_t` | 实际数值 |
| `limit` | `std::uint64_t` | 比较上限；`nonzero` 不使用此成员 |
| `constraint` | `constraint_kind` | 该数值必须满足的关系，默认 `at_most` |

### `invalid_entity_relation`

```cpp
struct invalid_entity_relation;
```

实体之间的关系不符合操作前提。嵌套枚举 `reason` 包含 `inactive_character`（要求出战但未出战）、`defeated_character`（要求存活但已战败）和 `same_character`（来源与目标角色必须不同）。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 相关字段路径 |
| `cause` | `reason` | 不符合要求的实体关系 |

### `insufficient_dice_argument`

```cpp
struct insufficient_dice_argument;
```

执行时玩家持有的骰子不能满足请求。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `player` | `player_id` | 被检查的玩家 |
| `requested` | `dice_counts` | 请求的各类骰子数量 |
| `available` | `dice_counts` | 检查时实际持有的各类骰子数量 |

## 类型别名

### `command_entity_id`

```cpp
using command_entity_id = std::variant<player_id, character_id, skill_id,
    attachment_id, hand_card_id, deck_card_id, hand_card_status_id,
    deck_card_status_id, support_id, summon_id, combat_status_id>;
```

命令诊断使用的实体 ID variant，保留参数原本的实体类别，供调用方通过 `std::get_if` 或 `std::visit` 读取。
