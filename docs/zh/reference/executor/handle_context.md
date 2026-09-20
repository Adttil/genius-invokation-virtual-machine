[givm](../../reference.md) / [执行](../executor.md) / **handle_context**

# givm::handle_context

定义于头文件 `<givm/executor.hpp>`

```cpp
class handle_context;
```

一次事件响应使用的上下文。定义可以读取当前牌桌、取得随机值，并提交已经登记的后续效果及其全部输入。

由执行器传给定义源的 `handle`，仅在本次响应调用期间有效。定义源不自行构造，也不得在响应结束后保存或使用本对象。

## 成员函数

| | |
| --- | --- |
| [`table`](handle_context/table.md) | 读取当前牌桌 |
| [`random`](handle_context/random.md) | 取得下一个随机值 |
| [`invoke`](handle_context/invoke.md) | 提交入口及其全部输入 |

## 参阅

| | |
| --- | --- |
| [定义源协议](../definition/source_protocol.md) | 事件响应与查询的定义方式 |
| [`program_entry`](../definition/program_entry.md) | 已登记效果的入口 |
