[givm](../../reference.md) / [通用工具](../utils.md) / **type_list_cat**

# givm::type_list_cat

定义于头文件 `<givm/utils/type_list.hpp>`

```cpp
template<class... TList>
using type_list_cat = /* 拼接后的类型序列 */;
```

多组类型序列按给定顺序拼接所得的 `type_list` 类型。

## 模板参数

|  |  |
| --- | --- |
| `TList...` | 要连接的 [`type_list`](type_list.md) 类型 |

## 示例

```cpp
#include <print>

#include <givm/utils/type_list.hpp>

int main()
{
    using joined = givm::type_list_cat<givm::type_list<int>, givm::type_list<double, char>>;
    std::println("合并后类型数量: {}", joined::size());
    std::println("char 的位置: {}", joined::index_of<char>());
}
```

输出

```text
合并后类型数量: 3
char 的位置: 2
```
