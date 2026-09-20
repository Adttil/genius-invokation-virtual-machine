[givm](../../reference.md) / [执行](../executor.md) / **program_invoker**

# givm::program_invoker

定义于头文件 `<givm/executor.hpp>`

```cpp
class program_invoker;
```

[`handle_context`](handle_context.md) 提交后续效果时使用的调用对象。响应先根据当前事件与牌桌确定效果输入，再把它们交给已经登记的程序执行；费用响应提交的效果则保留到确认支付时执行。

本对象由执行器准备，定义源通过 `context.invoke(...)` 提交，不自行构造或保存调用对象。

## 成员函数

| | |
| --- | --- |
| [`operator()`](program_invoker/operator_call.md) | 提交入口及其全部输入 |

## 参阅

| | |
| --- | --- |
| [定义源协议](../definition/source_protocol.md) | 事件响应与查询的定义方式 |
| [`program_entry`](../definition/program_entry.md) | 已登记效果的入口 |
