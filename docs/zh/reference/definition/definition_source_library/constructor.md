[givm](../../../reference.md) / [定义](../../definition.md) / [definition_source_library](../definition_source_library.md) / **(构造函数)**

# givm::definition_source_library::definition_source_library

定义于头文件 `<givm/definition.hpp>`

```cpp
definition_source_library() = default;
```

构造一个不包含实体定义的源库。

## 返回值

（无）

## 注意

声明中的默认构造函数由编译器隐式提供。

## 示例

```cpp
#include <print>

#include <givm/givm.hpp>

int main()
{
    givm::definition_source_library sources{};
    std::println("已登记恢复药剂: {}", sources.has<givm::card_definition>("恢复药剂"));
}
```

输出

```text
已登记恢复药剂: false
```
