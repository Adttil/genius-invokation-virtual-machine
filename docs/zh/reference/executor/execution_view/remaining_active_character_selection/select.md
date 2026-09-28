[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<remaining_active_character_selection>](../remaining_active_character_selection.md) / **select**

# givm::execution_view<execution_state::remaining_active_character_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
execution_state select(
    const definition_library& library, table& card_table, TRandom& random,
    character_id character
) const;
```
[`character_id`](../../../table/character_id.md)

提交剩余玩家选择的角色 ID 并推进。

## 模板参数

| | |
| --- | --- |
| `TRandom` | 非 `const`、非 `volatile` 的可调用对象类型，其无参数调用结果可隐式转换为 `std::uint32_t`。 |

## 参数

| | |
| --- | --- |
| `library` | 与当前执行现场配套的定义库。 |
| `card_table` | 当前对局牌桌。 |
| `random` | 本次推进使用的随机源，以左值传入。 |
| `character` | [`player()`](player.md) 对应玩家的一名有效角色的完整 ID |

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入不合法会抛出 [`view_input_error`](../../view_input_error.md)，尚未填写选择或开始推进。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

## 注意

所属玩家由当前现场确定。可先通过 [`selection_validate`](selection_validate.md) 独立检查。提交时立即填写选择并推进；Debug 自动执行输入检查，Release 由调用方保证输入合法。开始推进后，旧视图失效。