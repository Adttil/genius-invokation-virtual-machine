[givm](../../reference.md) / [牌桌](../table.md) / **definition_id**

# givm::definition_id

定义于头文件 `<givm/table.hpp>`

```cpp
template<class TEntityView>
using definition_id = issued_id<TEntityView>;
```

[`issued_id<TEntityView>`](issued_id.md) 的别名，表示一项实体定义的身份标识。它说明卡牌、角色或其他实体采用哪一份定义，不标识牌桌上的某一个实体。

## 模板参数

|  |  |
| --- | --- |
| `TEntityView` | 定义类别，取 [`definition_types`](../definition/definition_types.md) 中的类型 |

## 注意

卡牌使用 `definition_id<card_definition>`，卡牌状态使用 `definition_id<status_definition>`；其余类别使用对应的只读 view 类型。

默认构造得到无效 ID；有效 ID 由定义模块的编译和名称映射接口提供。ID 不持有定义库。有效只表示已经取得一个值，不检查该值属于哪份定义库；不同编译产物中的相同数值不保证代表同一定义。

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
| [`issued_id`](issued_id.md) | 按类别区分的已发行 ID |
| [`operator==`](issued_id/operator_equal.md) | 比较同一类别的 ID |
| [`issued_id_map`](../definition/issued_id_map.md) | 定义名称与 ID 的对应表 |
