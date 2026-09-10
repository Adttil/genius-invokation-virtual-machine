[givm](../../reference.md) / [定义](../definition.md) / **issued_id**

# givm::issued_id

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TTag>
class issued_id;
```

某个定义库中一项定义或标签的身份标识。不同类别的 ID 不能互换；同一种卡牌的不同牌桌实体可以共同使用同一个定义 ID。

## 模板参数

|  |  |
| --- | --- |
| `TTag` | 区分 ID 类别的类型 |

## 成员常量

|  |  |
| --- | --- |
| [`invalid_value`](issued_id/invalid_value.md) | 未取得 ID 时的无效值 |

## 成员函数

|  |  |
| --- | --- |
| [(构造函数)](issued_id/constructor.md) | 构造一个无效 ID |
| [`is_valid`](issued_id/is_valid.md) | 检查是否持有 ID |
| [`operator bool`](issued_id/operator_bool.md) | 检查是否持有 ID |
| [`value`](issued_id/value.md) | 取得 ID 的值 |
| [`set_invalid`](issued_id/set_invalid.md) | 清除 ID |

## 注意

有效表示该对象持有已分配的值，不检查它是否属于某个特定定义库。只能在发放该 ID 的映射及配套定义库中使用它。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    auto card = ids.add<givm::card_definition>("恢复药剂", { "治疗" });
    std::println("已取得卡牌定义 ID: {}", card.is_valid());
    card.set_invalid();
    std::println("清除后有效: {}", static_cast<bool>(card));
}
```

输出

```text
已取得卡牌定义 ID: true
清除后有效: false
```

## 参阅

|  |  |
| --- | --- |
| [`definition_id`](definition_id.md) | 实体定义的身份标识 |
| [`tag_id`](tag_id.md) | 分类标签的身份标识 |
