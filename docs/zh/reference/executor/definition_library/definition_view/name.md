[givm](../../../../reference.md) / [执行](../../../executor.md) / [definition_library](../../definition_library.md) / [definition_view](../definition_view.md) / **name**

# givm::definition_library::definition_view::name

定义于头文件 `<givm/executor.hpp>`

```cpp
std::string_view name() const;
```

取得该实体定义的名称。

## 返回值

定义名称。

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
        std::tuple{}, std::tuple{ givm::start_round{ .max_rounds = 1 } }, givm::compile_mode::normal
    );
    const auto card = ids.get_id<givm::card_definition>("恢复药剂");
    std::println("定义名称: {}", library[card].name());
}
```

输出

```text
定义名称: 恢复药剂
```
