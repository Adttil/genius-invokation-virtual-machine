[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **is_control**

# givm::definition_library::is_control

定义于头文件 `<givm/executor.hpp>`

```cpp
bool is_control(definition_id<attachment_view> id) const noexcept;
```

检查一种角色附属定义是否属于控制效果。冻结、石化等限制角色使用技能的附属可通过 `control` 标签声明这一分类。

## 参数

| | |
| --- | --- |
| `id` | 本定义库中的有效附属定义 ID |

## 返回值

定义具有 `control` 标签时返回 `true`，否则返回 `false`。

## 注意

本函数检查定义分类，不表示牌桌上已有该附属。检查某个角色当前是否受控，使用 [`is_controlled`](is_controlled.md)。带 `control` 标签的附属会受 [`is_control_immune`](is_control_immune.md) 所描述的免控规则限制。
