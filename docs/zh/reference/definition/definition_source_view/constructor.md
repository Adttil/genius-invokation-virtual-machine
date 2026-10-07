[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_view](../definition_source_view.md) / **(构造函数)**

# givm::definition_source_view::definition_source_view

定义于头文件 `<givm/definition_source_interface.hpp>`

```cpp
template<class TSource>
    requires (TSource::category == TCategory)
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

构造时检查[动态定义源](../source_protocol.md#动态定义源)所属类别的完整接口：每个可订阅事件和支持的查询都必须同时具有能力判断与签名正确的实现，能力判断返回 `false` 的分支也不例外。动态历史摘要须完整提供 `can_handle<Event>()` 和摘要 `handle`。缺少接口或返回类型错误属于 C++ 编译错误。静态定义源仍可省略不响应的事件及采用默认行为的查询。

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
    std::println("依赖的支援: {}", view.dependencies<givm::definition_category::support>().front());
}
```

输出

```text
定义源名称: 召唤卡
依赖的支援: 协助者
```
