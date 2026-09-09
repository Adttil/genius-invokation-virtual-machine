[givm](../../../reference.md) / [主题](../../topic.md) / [type](../type.md) / **function**

# givm::type::function

定义于头文件 `<givm/type.hpp>`

```cpp
template<class T>
T function(T arg1, type2 arg2 = constant) const;
```
[`type2`](相对路径到/type2.md)
[`constant1`](相对路径/constant1.md)

成员函数做什么。

可选的补充说明。

## 模板参数

| | |
| --- | --- |
| `T` | 模板参数是什么和可选的要求限制 |

## 参数

| | |
| --- | --- |
| `arg1` | 参数是什么 |
| `arg2` | 参数是什么 |

## 返回值

返回值是什么。

## 异常

| | |
| --- | --- |
| `error1` | 什么情况下抛出 |

## 注意

注意事项。

## 示例
可选的说明，通常不写，优先使用注释说明。
```cpp
#include <givm/header.hpp>

#include <iostream>

int main()
{
    // 可选，通过注释补充说明
    givm::type object{};
    auto result = object.function(1, {});
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
| [`constant1`](相对路径到/constant1.md) | 相关常量是什么 |
| [`function2`](function2.md) | 相关成员函数做什么 |
| [`type2::function`](相对路径到/type2/function.md) | 相关成员函数做什么 |