[givm](../../../reference.md) / [执行](../../executor.md) / [random_fn](../random_fn.md) / **operator()**

# givm::random_fn::operator()

定义于头文件 `<givm/executor.hpp>`

```cpp
std::uint32_t operator()() const;
```

从所接入的随机源取得下一个值。

## 返回值

随机源本次产生的值，转换为 `std::uint32_t`。

## 注意

可能改变原随机源的状态，即使 [`random_fn`](../random_fn.md) 自身为 const。随机源抛出的异常会传递给调用方。

[费用响应](../../definition/events/cost_of_switch.md)不得使用随机数；在费用响应中调用本函数属于未定义行为。

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
