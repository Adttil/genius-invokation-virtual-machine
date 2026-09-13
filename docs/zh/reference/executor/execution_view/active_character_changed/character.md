[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<active_character_changed>](../active_character_changed.md) / **character**

# givm::execution_view<execution_state::active_character_changed>::character

定义于头文件 `<givm/executor.hpp>`

```cpp
character_id character() const noexcept;
```
[`character_id`](../../../table/character_id.md)

取得本次将设置为出战的角色。

## 返回值

将要设置为出战角色的角色 ID，自身携带所属玩家。该玩家在牌桌上的出战角色仍为原值；下一次推进才更新。
