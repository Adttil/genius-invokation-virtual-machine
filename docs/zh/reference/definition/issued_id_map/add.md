[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **add**

# givm::issued_id_map::add

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TDefinition>
definition_id<TDefinition> add(std::string_view name, std::initializer_list<std::string_view> tags); // (1)

template<class TDefinition, std::ranges::input_range TTags>
    requires std::convertible_to<std::ranges::range_reference_t<TTags>, std::string_view>
definition_id<TDefinition> add(std::string_view name, TTags&& tags); // (2)
```

为一个实体定义登记名称和标签，取得之后查询或引用它所用的 ID。

## 模板参数

|  |  |
| --- | --- |
| `TDefinition` | 定义类别，见 [`definition_types`](../definition_types.md) |
| `TTags` | 元素可转换为 `std::string_view` 的输入范围 |

## 参数

|  |  |
| --- | --- |
| `name` | 本类别中尚未登记的定义名称 |
| `tags` | 该定义具有的标签；标签必须已经登记 |

## 返回值

新分配的定义 ID。

## 注意

名称与标签的字符存储必须保持有效。此操作只登记映射，不向已编译的定义库添加定义。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗" };
    const auto card = ids.add<givm::card_definition>("恢复药剂", { "治疗" });
    std::println("新 ID 有效: {}", card.is_valid());
    std::println("已登记定义: {}", ids.has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
新 ID 有效: true
已登记定义: true
```
