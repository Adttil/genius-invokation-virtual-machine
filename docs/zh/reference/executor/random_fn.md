[givm](../../reference.md) / [执行](../executor.md) / **random_fn**

# givm::random_fn

定义于头文件 `<givm/executor.hpp>`

```cpp
class random_fn;
```

指令和事件响应取得随机值的统一入口。实际随机序列由调用方提供的随机源决定。

## 成员函数

|  |  |
| --- | --- |
| [`(构造函数)`](random_fn/constructor.md) | 接入一个随机源 |
| [`operator()`](random_fn/operator_call.md) | 取得下一个随机值 |

## 注意

不拥有随机源，复制也不会保存独立的随机序列状态。所接入对象须在使用期间保持有效；不同构造方式的生命周期约定见[构造函数](random_fn/constructor.md)。

## 示例

```cpp
#include <print>
#include <cstdint>

#include <givm/givm.hpp>

int main()
{
    std::uint32_t next = 7;
    auto source = [&]() -> std::uint32_t { return next++; };
    givm::random_fn random{ source };
    std::println("第一次取得的值: {}", random());
    std::println("第二次取得的值: {}", random());
}
```

输出

```text
第一次取得的值: 7
第二次取得的值: 8
```
