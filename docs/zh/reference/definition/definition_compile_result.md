[givm](../../reference.md) / [定义](../definition.md) / **definition_compile_result**

# givm::definition_compile_result

定义于头文件 `<givm/definition.hpp>`

```cpp
struct definition_compile_result;
```

准备对局所得的定义库和配套 ID 映射。两者描述同一组选定定义，可以一起用于按名称准备牌组和创建牌桌。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `library` | [`definition_library`](definition_library.md) | 对局使用的实体定义与流程 |
| `id_map` | [`issued_id_map`](issued_id_map.md) | 这些定义及标签的名称映射 |

## 示例

```cpp
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view source_name;

    std::string_view name() const { return source_name; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source potion{ "恢复药剂" };
    givm::definition_source_library sources{};
    std::println("登记成功: {}", sources.add(potion));
    const auto [library, ids] = sources.compile(
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    std::println("可用卡牌: {}", library.name(ids.get_id<givm::card_definition>("恢复药剂")));
}
```

输出

```text
登记成功: true
可用卡牌: 恢复药剂
```
