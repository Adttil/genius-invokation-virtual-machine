[givm](../../reference.md) / [通用工具](../utils.md) / **maybe_mutable**

# givm::maybe_mutable

定义于头文件 `<givm/utils/maybe_const.hpp>`

```cpp
template<bool Mutable, class T>
using maybe_mutable = std::conditional_t<Mutable, T, const T>;
```

由访问权限决定 const 限定的类型。

## 模板参数

|  |  |
| --- | --- |
| `Mutable` | 可修改标志；为 true 时类型为 `T`，否则为 `const T` |
| `T` | 原类型 |

## 示例

```cpp
#include <print>

#include <givm/utils/maybe_const.hpp>

int main()
{
    int value = 3;
    givm::maybe_mutable<true, int>& writable = value;
    givm::maybe_mutable<false, int>& readable = value;
    writable = 7;
    std::println("只读访问得到的值: {}", readable);
}
```

输出

```text
只读访问得到的值: 7
```
