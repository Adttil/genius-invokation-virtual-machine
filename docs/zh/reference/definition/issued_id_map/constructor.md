[givm](../../../reference.md) / [定义](../../definition.md) / [issued_id_map](../issued_id_map.md) / **(构造函数)**

# givm::issued_id_map::issued_id_map

定义于头文件 `<givm/definition.hpp>`

```cpp
explicit issued_id_map(std::initializer_list<std::string_view> tags); // (1)

template<std::ranges::input_range TTags>
    requires std::convertible_to<std::ranges::range_reference_t<TTags>, std::string_view>
explicit issued_id_map(TTags&& tags); // (2)
```

构造一个尚未包含实体定义的映射，并登记之后可以使用的标签。重复标签只登记一次。

## 模板参数

|  |  |
| --- | --- |
| `TTags` | 元素可转换为 `std::string_view` 的输入范围 |

## 参数

|  |  |
| --- | --- |
| `tags` | 全部可用的标签名称 |

## 返回值

（无）

## 注意

标签名称的字符存储必须保持有效。标签集合在构造后不能扩充。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::issued_id_map ids{ "治疗", "治疗", "料理" };
    std::println("标签数量: {}", ids.tag_names().size());
    std::println("已有卡牌定义: {}", ids.has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
标签数量: 2
已有卡牌定义: false
```
