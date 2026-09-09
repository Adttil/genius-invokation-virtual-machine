[givm](../../reference.md) / [主题](../topic.md) / **constant**

# givm::constant

定义于头文件 `<givm/constant.hpp>`

```cpp
template<class T>
inline constexpr T constant = T{42};
```

这个常量是什么。

可选的补充说明。

## 模板参数

| | |
| --- | --- |
| `T` | 模板参数是什么和可选的要求限制 |

## 注意

注意事项。

## 示例

```cpp
#include <givm/constant.hpp>

#include <iostream>

int main()
{
    std::cout << "constant: " << givm::constant<int> << '\n';
}
```

输出

```text
constant: 42
```

## 参阅

| | |
| --- | --- |
| [`type2`](相对路径到/type2.md) | 相关类型是什么 |
| [`function2`](相对路径到/function2.md) | 相关函数做什么 |
