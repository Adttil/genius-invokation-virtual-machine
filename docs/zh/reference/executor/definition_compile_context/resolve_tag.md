[givm](../../../reference.md) / [执行](../../executor.md) / [definition_compile_context](../definition_compile_context.md) / **resolve_tag**

# givm::definition_compile_context::resolve_tag

定义于头文件 `<givm/executor.hpp>`

```cpp
tag_id resolve_tag(std::string_view name) const;
```

取得效果需要识别的分类标签。源可以声明使用某个标签，即使这次选中的其他定义都不具有该标签。

## 参数

|  |  |
| --- | --- |
| `name` | 本源在 `tag_dependencies()` 中声明的标签名称 |

## 返回值

配套定义库中的标签 ID。

## 异常

|  |  |
| --- | --- |
| `std::invalid_argument` | 本源未声明该标签依赖，或标签不存在 |

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>
#include <tuple>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;
    std::string_view name() const { return "治疗检索"; }
    auto tag_dependencies() const
    { return std::array<std::string_view, 1>{ "治疗" }; }

    givm::tag_id compile(givm::definition_compile_context& context) const
    {
        const auto tag = context.resolve_tag("治疗");
        std::println("已取得治疗标签: {}", tag.is_valid());
        return tag;
    }
};

int main()
{
    const card_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::normal
    );
}
```

输出

```text
已取得治疗标签: true
```
