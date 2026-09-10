[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_view](../definition_source_view.md) / **(构造函数)**

# givm::definition_source_view::definition_source_view

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class TSource>
constexpr definition_source_view(const TSource& source);
```

为一份定义源建立只读视图。

## 模板参数

|  |  |
| --- | --- |
| `TSource` | 符合[定义源协议](../source_protocol.md)的类型 |

## 参数

|  |  |
| --- | --- |
| `source` | 属于 `TCategory` 类别的定义源对象 |

## 返回值

（无）

## 注意

不复制或拥有源对象；不要从即将销毁的临时源对象创建长期使用的视图。

## 示例

```cpp
#include <array>
#include <print>
#include <string_view>

#include <givm/givm.hpp>

struct card_source
{
    using definition_category = givm::card_definition;

    std::string_view name() const { return "召唤卡"; }
    auto tags() const { return std::array<std::string_view, 1>{ "召唤" }; }
    auto support_dependencies() const
    { return std::array<std::string_view, 1>{ "协助者" }; }
    auto tag_dependencies() const
    { return std::array<std::string_view, 1>{ "治疗" }; }
    auto card_dependencies_by_tag() const
    { return std::array<std::string_view, 1>{ "治疗 & !料理" }; }
    int compile(givm::definition_compile_context&) const { return 0; }
};

int main()
{
    const card_source source{};
    const givm::definition_source_view<givm::card_definition> view{ source };
    std::println("定义源名称: {}", view.name());
    std::println("依赖的支援: {}", view.dependencies<givm::support_view>().front());
}
```

输出

```text
定义源名称: 召唤卡
依赖的支援: 协助者
```
