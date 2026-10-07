[givm](../../../reference.md) / [牌桌](../../table.md) / [skill_id](../skill_id.md) / **operator==**

# givm::skill_id::operator==

定义于头文件 `<givm/table.hpp>`

```cpp
friend constexpr bool operator==(skill_id, skill_id) noexcept = default;
```

比较同类 ID 的身份，不检查实体是否在场，也不区分属于哪张牌桌。
