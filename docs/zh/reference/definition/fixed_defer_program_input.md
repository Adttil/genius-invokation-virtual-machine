[givm](../../reference.md) / [定义](../definition.md) / **fixed_defer_program_input**

# givm::fixed_defer_program_input

定义于头文件 `<givm/definition.hpp>`

```cpp
class fixed_defer_program_input;
```

固定延迟命令使用的编译用参数，拥有目标程序入口、参数快照以及校验所需的描述。通过 [`fixed_defer_invoke`](fixed_defer_invoke.md) 准备，无需自行填写字节或校验信息。

默认构造表示空入口；放入 `defer_program` 时表示该命令改用动态输入。复制会复制参数和描述，移动可转移其所有权。

## 成员函数

| | |
| --- | --- |
| [`entry`](fixed_defer_program_input/entry.md) | 取得目标入口 |
| [`bytes`](fixed_defer_program_input/bytes.md) | 取得已打包参数的只读字节视图 |

编译时检查参数协议，只将入口和参数字节写入程序。校验描述不成为运行期的参数；动态延迟输入使用 [`defer_program_input`](command_inputs/defer_program_input.md)。
