[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<dice_selection>](../dice_selection.md) / **remaining**

# givm::execution_view<execution_state::dice_selection>::remaining

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr std::uint32_t remaining() const noexcept;
constexpr std::uint32_t remaining(player_id player) const noexcept;
```
[`player_id`](../../../table/player_id.md)

取得当前填写的玩家或指定玩家剩余的重投次数。

## 参数

| | |
| --- | --- |
| `player` | 要查询剩余重投机会的玩家。 |

## 返回值

相应玩家尚可使用的重投次数。
