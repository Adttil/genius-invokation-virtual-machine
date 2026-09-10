[givm](../../reference.md) / [定义](../definition.md) / **definition_id**

# givm::definition_id

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TEntityView>
using definition_id = issued_id<TEntityView>;
```

标识一项实体定义。它说明卡牌、角色或其他实体采用哪一份定义，不标识牌桌上的某一个实体。

## 模板参数

|  |  |
| --- | --- |
| `TEntityView` | 定义类别，取 [`definition_types`](definition_types.md) 中的类型 |

## 注意

卡牌使用 `definition_id<card_definition>`，卡牌状态使用 `definition_id<status_definition>`；其余类别使用对应的只读 view 类型。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{};
    const givm::definition_id<givm::card_definition> card =
        ids.add<givm::card_definition>("恢复药剂", {});
    std::println("已取得卡牌定义: {}", card.is_valid());
}
```

输出

```text
已取得卡牌定义: true
```

## 参阅

|  |  |
| --- | --- |
| [`issued_id`](issued_id.md) | ID 的共同操作 |
