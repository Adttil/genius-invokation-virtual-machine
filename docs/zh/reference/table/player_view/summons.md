[givm](../../../reference.md) / [牌桌](../../table.md) / [player_view](../player_view.md) / **summons**

# givm::player_view::summons

定义于头文件 `<givm/table.hpp>`

```cpp
template<bool SkipErased = true>
constexpr auto summons() const;
```

遍历该玩家的召唤物。

## 模板参数

|  |  |
| --- | --- |
| `SkipErased` | 是否跳过已经移除的实体，默认为 true |

## 返回值

产生 [`summon_view`](../summon_view.md) 的范围。

## 注意

遍历所得访问对象的存活约定见[实体的身份与访问](../entity_access.md)。
