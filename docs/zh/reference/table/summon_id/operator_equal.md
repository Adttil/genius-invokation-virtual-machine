[givm](../../../reference.md) / [牌桌](../../table.md) / [summon_id](../summon_id.md) / **operator==**

# givm::summon_id::operator==

定义于头文件 `<givm/table.hpp>`

```cpp
friend constexpr bool operator==(summon_id, summon_id) noexcept = default;
```

比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌。
