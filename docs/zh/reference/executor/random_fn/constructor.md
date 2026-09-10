[givm](../../../reference.md) / [执行](../../executor.md) / [random_fn](../random_fn.md) / **random_fn**

# givm::random_fn::random_fn

定义于头文件 `<givm/executor.hpp>`

```cpp
template<class TRandom>
    requires std::invocable<TRandom&>
        && std::convertible_to<std::invoke_result_t<TRandom&>, std::uint32_t>
constexpr explicit random_fn(TRandom& random) noexcept;
```

构造一个引用调用方随机源的视图。

## 模板参数

|  |  |
| --- | --- |
| `TRandom` | 非 const、非 volatile 对象类型，左值可无参数调用，结果可隐式转换为 `std::uint32_t` |

## 参数

|  |  |
| --- | --- |
| `random` | 要使用的随机源对象，须比本次使用活得更久 |

## 返回值

（无）

## 注意

`TRandom` 也可以是 [`random_fn`](../random_fn.md)。从非 const 的 [`random_fn`](../random_fn.md) 左值构造时，调用此模板构造函数，接入的是该包装对象本身；使用新对象期间，原包装对象及其底层随机源都须保持有效。

通过 const 的 [`random_fn`](../random_fn.md) 左值进行复制构造时，两个包装对象接入同一个随机源，原包装对象可以先于副本销毁；底层随机源仍须保持有效。

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
