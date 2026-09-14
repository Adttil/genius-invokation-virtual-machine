[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **has_tag**

# givm::definition_library::definition_view::has_tag

定义于头文件 `<givm/executor.hpp>`

```cpp
bool has_tag(tag_id tag) const;
```

检查该定义是否属于给定标签所表示的分类。

## 参数

|  |  |
| --- | --- |
| `tag` | 配套定义库中的有效标签 ID |

## 返回值

具有该标签时返回 `true`，否则返回 `false`。

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

    std::string_view name() const { return "恢复药剂"; }
    auto tags() const { return std::array<std::string_view, 1>{ "治疗" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    givm::definition_source_library sources{};
    sources.add(source);
    const auto [library, ids] = compile(
        sources,
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    std::println("属于治疗牌: {}", library[card].has_tag(ids.get_tag_id("治疗")));
}
```

输出

```text
属于治疗牌: true
```
