[givm](../../reference.md) / [主题](../topic.md) / **enum_type**

# givm::enum_type

定义于头文件 `<givm/enum_type.hpp>`

```cpp
enum class enum_type
{
    value1,
    value2
};
```

这个枚举类型是什么。

可选的补充说明。

## 枚举值

| | |
| --- | --- |
| `value1` | `value1` 代表什么 |
| `value2` | `value2` 代表什么 |

## 注意

注意事项。

## 示例

```cpp
#include <givm/enum_type.hpp>

#include <iostream>

int main()
{
    const auto value = givm::enum_type::value1;
    std::cout << std::boolalpha
              << "is value1: " << (value == givm::enum_type::value1) << '\n';
}
```

输出

```text
is value1: true
```

## 参阅

| | |
| --- | --- |
| [`type2`](相对路径到/type2.md) | 相关类型是什么 |
| [`function2`](相对路径到/function2.md) | 相关函数做什么 |
