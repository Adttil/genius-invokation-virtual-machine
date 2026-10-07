[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_view](../definition_source_view.md) / **name**

# givm::definition_source_view::name

定义于头文件 `<givm/definition_source_interface.hpp>`

```cpp
std::string_view name() const;
```

取得定义源的名称。

## 返回值

源对象返回的名称。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    static constexpr auto category = givm::definition_category::card;

    std::string_view name() const { return "召唤卡"; }
    auto tags() const { return std::array<std::string_view, 1>{ "召唤" }; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "协助者" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const givm::definition_source_view<givm::definition_category::card> view{ source };
    std::println("定义源名称: {}", view.name());
}
```

输出

```text
定义源名称: 召唤卡
```
