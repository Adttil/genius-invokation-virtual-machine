[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<initial_active_character_selection>](../initial_active_character_selection.md) / **select**

# givm::execution_view<execution_state::initial_active_character_selection>::select

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void select(character_id character) const noexcept;
```
[`character_id`](../../../table/character_id.md)

填写首先提交的出战角色选择。

## 参数

| | |
| --- | --- |
| `character` | 首先选择的有效角色，可属于任意一方 |

## 返回值

(无)

## 注意

可先通过 [`check_selection`](check_selection.md) 独立检查。本操作不自动检查，只填写选择，下一次推进才消费输入；另一方的出战角色随后再选择。
