[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **card_count**

# givm::execution_view<execution_state::action_selection>::card_count

定义于头文件 `<givm/runtime.hpp>`

```cpp
constexpr std::size_t card_count() const noexcept(/* Release 为 true，Debug 为 false */);
```

取得当前可选手牌的数量，供上层列出出牌和元素调和共用的手牌候选。

## 返回值

当前行动玩家仍在手中的有效手牌数量。候选按手牌遍历顺序从零开始编号，可通过 [`card_id`](card_id.md) 查询对应手牌。

## 注意

Debug 下，视图不属于当前现场或已经失效时抛出 [`execution_view_error`](../../execution_view_error.md)；Release 保持 `noexcept` 且不检查这些条件。

无需先计算费用。本操作不检查目标或支付，也不提交行动。候选索引仅用于当前行动现场，进入下一次行动现场后须重新取得候选。
