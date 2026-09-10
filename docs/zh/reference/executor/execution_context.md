[givm](../../reference.md) / [执行](../executor.md) / **execution_context**

# givm::execution_context

定义于头文件 `<givm/executor.hpp>`

```cpp
class execution_context;
```

供核心指令处理对局推进的上下文。核心指令通过它选择继续处理、等待外部输入、进入另一段效果或结束对局。

## 成员函数

|  |  |
| --- | --- |
| [`enter_next`](execution_context/enter_next.md) | 继续执行下一条指令 |
| [`yield_next`](execution_context/yield_next.md) | 暂停并在下次执行下一条指令 |
| [`yield`](execution_context/yield.md) | 暂停并在下次重新执行本条指令 |
| [`enter`](execution_context/enter.md) | 进入另一段效果 |
| [`end_game`](execution_context/end_game.md) | 结束对局 |
| [`stack`](execution_context/stack.md) | 访问临时数据 |
| [`current_stage`](execution_context/current_stage.md) | 访问本条指令记录的进度 |

## 注意

执行器在调用核心指令时提供此对象。定义源通过组合[核心给定的指令](../instructions.md)安排效果，不能借此扩展指令集合；对局的外层驱动使用 [`executor`](executor.md)。
