[givm](../../reference.md) / [主题](../topic.md) / **type_alias**

# givm::type_alias

定义于头文件 `<givm/type_alias.hpp>`

```cpp
template<class T>
using type_alias = type2<T>;
```
[`type2`](相对路径到/type2.md)

这个类型别名是什么。

可选的补充说明。

## 模板参数

| | |
| --- | --- |
| `T` | 模板参数是什么和可选的要求限制 |

## 注意

注意事项。

## 示例

```cpp
#include <givm/type_alias.hpp>

#include <iostream>

int main()
{
    givm::type_alias<int> object{};
    auto result = object.method1();
    std::cout << "result: " << result << '\n';
}
```

输出

```text
result: 实际结果
```

## 参阅

| | |
| --- | --- |
| [`type2`](相对路径到/type2.md) | 相关类型是什么 |
| [`function2`](相对路径到/function2.md) | 相关函数做什么 |
