[givm](../../../reference.md) / [牌桌](../../table.md) / [card_table](../card_table.md) / **definition_library**

# givm::card_table::definition_library

定义于头文件 `<givm/table.hpp>`

```cpp
constexpr const definition_library_type& definition_library() const;
```

取得这场对局使用的实体定义库。

## 返回值

构造时传入的 [`definition_library`](../../definition/definition_library.md) 的只读引用。

## 示例

```cpp
#include <print>
#include <tuple>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    const auto [library, id_map] = sources.compile(std::tuple{}, std::tuple{});
    givm::card_table table{ library };
    std::println("使用指定的定义库: {}", &table.definition_library() == &library);
}
```

输出

```text
使用指定的定义库: true
```
