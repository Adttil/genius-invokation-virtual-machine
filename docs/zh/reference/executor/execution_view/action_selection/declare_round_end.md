[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **declare_round_end**

# givm::execution_view<execution_state::action_selection>::declare_round_end

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<class TRandom>
execution_state declare_round_end(
    const definition_library& library, table& card_table, TRandom& random
) const;
```

提交当前玩家的结束回合声明并推进。

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

## 返回值

推进后到达的 [`execution_state`](../../execution_state.md)，可能是下一处输入、观察或终局现场。

## 异常

未定义 `NDEBUG` 时，现场已失效或种类错误会抛出 [`execution_view_error`](../../execution_view_error.md)；输入合法性结果失败时抛出 [`view_input_error`](../../view_input_error.md)；实体 ID 越界或已移除的诊断沿用 [`command_input_error`](../../command_input_error.md)。这些检查均在填写选择和开始推进之前。进入执行后，定义源、随机源及命令检查产生的异常向外传播，执行不提供回滚。

## 注意

本操作无需支付参数，提交后立即执行结束声明并推进。开始推进后，旧视图失效。
