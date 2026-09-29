[givm](../../../reference.md) / [执行](../../executor.md) / [execution_view](../execution_view.md) / **execution_view<initialized>**

# givm::execution_view<execution_state::initialized>

定义于头文件 `<givm/runtime.hpp>`

```cpp
template<execution_state State>
class execution_view;
```

本页说明 `State == execution_state::initialized` 时的视图。执行器与历史摘要已经初始化、游戏流程尚未开始执行的现场视图。由 [`executor::start`](../executor/start.md) 返回。

## 成员函数

| | |
| --- | --- |
| [`resume`](resume.md) | 执行至首个输入、观察或终局现场。 |

## 注意

可在此现场读取初始化结果，或复制牌桌与执行器用于独立分支。复制执行器后，从副本重新取得此视图；原视图仍借用原执行器。
