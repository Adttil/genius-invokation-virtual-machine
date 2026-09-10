[givm](../../../reference.md) / [执行](../../executor.md) / [execution_context](../execution_context.md) / **end_game**

# givm::execution_context::end_game

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr bool end_game(game_result result) noexcept;
```

以给定结果结束对局。

## 参数

|  |  |
| --- | --- |
| `result` | 终局结果，不能为 `game_result::no_result` |

## 返回值

`false`。
