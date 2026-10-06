[givm](../../../../reference.md) / [执行](../../../executor.md) / [active_character_selection](../active_character_selection.md) / **select**

# givm::execution_view<execution_state::active_character_selection>::select

定义于头文件 `<givm/runtime.hpp>`。

```cpp
template<class TRandom>
execution_state select(const definition_library& library, table& table, TRandom& random,
    character_id character) const;
```

提交角色 ID。双方都需要选择时，第一个答案暂存，返回另一方的选择现场；收齐答案之前不更新出战位置或处理切人通知。收齐后按当前行动方、另一方的顺序逐个实际切换，并完整处理各自切人通知，再继续原程序。

返回下一暂停现场或 finished；原视图随推进失效。Debug 下不合法输入抛出 view_input_error，Release 要求调用方保证合法。

Debug 下视图不属于当前现场或已经失效时抛出 execution_view_error。
