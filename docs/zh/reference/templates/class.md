[givm](../../reference.md) / [主题](../topic.md) / **type**

# givm::type

定义于头文件 `<givm/type.hpp>`

```cpp
template<class T>
class type;
```

这个类型是什么。

可选的其他说明，帮助理解这个类有什么作用和如何使用。

## 模板参数

| | |
| --- | --- |
| `T` | T是什么，可选：其要求和限制 |

## 成员类型

| | |
| --- | --- |
| `member_type` | 成员类型是什么 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `x` | `int` | `x` 是什么 |
| `y` | `float` | `y` 是什么 |

## 成员函数

| | |
| --- | --- |
| [(构造函数)](type/constructor.md) | 构造 type |
| [(析构函数)](type/destructor.md) | 析构 type |
| [`method1`](type/method1.md) | `method1` 做什么 |
| [`method2`](type/method2.md) | `method2` 做什么 |

### 分类一

| | |
| --- | --- |
| [`method3`](type/method3.md) | `method3` 做什么 |
| [`method4`](type/method4.md) | `method4` 做什么 |

## 非成员函数

| | |
| --- | --- |
| [`function1`](相对路径到/function1.md) | `function1` 做什么 |

## 注意

注意事项。

## 示例
可选的说明，通常不写，优先使用注释说明。
```cpp
#include <givm/type.hpp>

#include <iostream>

int main()
{
    // 可选，通过注释补充说明
    givm::type<int> object{};
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
