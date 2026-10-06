[givm](../../reference.md) / [执行](../executor.md) / **视图输入错误原因**

# 视图输入错误原因

定义于头文件 `<givm/runtime.hpp>`

[`view_input_error<Reason>`](view_input_error.md) 保留每种操作的具体错误类型；本页列出新增的结构化原因，不把它们合并为通用枚举或 variant。

## 类

### givm::view_index_out_of_range

```cpp
struct view_index_out_of_range;
```

候选或玩家索引越界。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `field` | `std::string` | 索引字段名称 |
| `index` | `std::size_t` | 实际传入的索引 |
| `count` | `std::size_t` | 可用元素数量 |

### givm::invalid_card_positions

```cpp
struct invalid_card_positions;
```

换牌选择包含不存在的手牌位置。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `selected` | `std::bitset<selection_capacity>` | 本次换牌选择 |
| `card_count` | `std::size_t` | 当前有效手牌数量 |

### givm::action_cost_cache_error

```cpp
struct action_cost_cache_error;
```

行动费用报价的使用状态错误。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `action` | `std::string` | 行动类别名称 |
| `index` | `std::size_t` | 报价错误中的候选索引，特技使用零；缓存标识错误中的诊断值不能作为候选索引使用 |
| `cause` | `action_cost_cache_error::reason` | `expired_window` 表示标识不属于当前窗口缓存，`already_calculated` 表示重复报价，`incomplete_calculation` 表示先前报价未完整完成 |

### givm::action_target_validation_error

```cpp
struct action_target_validation_error;
```

提交行动时，某个目标前缀不合法，或当前选择不能完成。

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `checked_count` | `std::size_t` | 当前检查的目标前缀长度 |
| `target_count` | `std::size_t` | 本次采用的目标总数，至多两个 |
| `result` | `target_validation` | 定义查询返回的验证结果 |

## 枚举

### givm::action_unavailable

```cpp
enum class action_unavailable : std::uint8_t;
```

行动不可用的原因。

| | |
| --- | --- |
| `controlled` | 出战角色受控，不能使用技能或特技 |
| `missing_technique` | 当前出战角色没有可用特技 |
| `elemental_tuning_forbidden` | 选中的牌不允许元素调和 |

## 其他原因

支付、初始角色选择、初始换牌、显式玩家重投和调和骰子检查直接使用相应 `*_validation` 枚举；持有骰子不足可使用 [`insufficient_dice_argument`](command_input_error_reason.md)。各具体原因均支持 [`error_string`](error_string.md)。
