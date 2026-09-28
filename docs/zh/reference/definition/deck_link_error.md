[givm](../../reference.md) / [定义](../definition.md) / **deck_link_error**

# givm::deck_link_error

定义于头文件 `<givm/definition.hpp>`

```cpp
struct deck_link_error;
```

牌组中的名称无法解析为定义 ID 的诊断。

## 成员类型

| | |
| --- | --- |
| `definition_kind` | 枚举 `card` 与 `character`，表示卡牌或角色名称 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `kind` | `definition_kind` | 缺失定义的类别 |
| `index` | `std::size_t` | 对应类别的名称输入下标，从零开始 |
| `name` | `std::string` | 未找到的定义名称 |

## 参阅

| | |
| --- | --- |
| [`link_deck`](link_deck.md) | 按名称链接牌组 |
