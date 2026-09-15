[givm](../../../../reference.md) / [执行](../../../executor.md) / [execution_view<action_selection>](../action_selection.md) / **declare_round_end**

# givm::execution_view<execution_state::action_selection>::declare_round_end

定义于头文件 `<givm/executor.hpp>`

```cpp
constexpr void declare_round_end() const noexcept;
```

填写当前玩家宣布结束回合的请求。

## 返回值

(无)

## 注意

本操作无需支付参数，下一次推进才执行结束声明。
