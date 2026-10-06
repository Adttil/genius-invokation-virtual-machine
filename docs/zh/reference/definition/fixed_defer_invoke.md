[givm](../../reference.md) / [定义](../definition.md) / **fixed_defer_invoke**

# givm::fixed_defer_invoke

定义于头文件 `<givm/definition.hpp>`

```cpp
template<class... T>
fixed_defer_program_input fixed_defer_invoke(program_entry entry, const T&... inputs);
```

为固定延迟命令准备入口和参数，同时保留编译检查所需的描述。它复制参数中的数组内容和嵌套参数，原输入可以在返回后销毁。

`entry` 是目标程序入口。`inputs` 按目标程序动态命令的顺序提供；每个参数是核心命令的专用输入对象，嵌套延迟输入使用本函数的返回值。目标程序不需要参数时使用 `fixed_defer_invoke(entry)`。

返回的 [`fixed_defer_program_input`](fixed_defer_program_input.md) 用于 `defer_program{fixed_defer_invoke(entry, inputs...)}`。有多层延迟时可写成 `fixed_defer_invoke(parent, fixed_defer_invoke(child, child_input))`。

本函数不访问定义库。参数协议由编译器检查，Debug 和 Release 均执行；失败时通过 [`fixed_program_input_error`](../executor/fixed_program_input_error.md) 返回诊断。编译成功后只复制入口和参数字节，不将这份参数描述写入固定程序。

响应期间准备动态延迟输入使用 [`defer_invoke`](defer_invoke.md)。运行期打包对象不必保留编译用描述。
