[givm](../../../reference.md) / [定义](../../definition.md) / [事件](../events.md) / **history_summary_initialization**

# givm::history_summary_initialization

定义于头文件 `<givm/definition.hpp>`

```cpp
struct history_summary_initialization {
    static constexpr event_category category = event_category::normal;
};
```

仅供[历史摘要](../history_summary.md)订阅的初始化事件，没有成员。

[`executor::start`](../../executor/executor/start.md) 准备摘要状态空间后同步发送本事件。摘要通过通常的 `handle` 接口，使用 `history_summary_state` 修改自身字段，并可读取牌桌与定义库。通常先完成双方 [`load_deck`](../../executor/load_deck.md)，让初始化响应能读取完整的初始牌桌。

库不保证摘要字段初始为零。需要初值的摘要应响应本事件写入初值；省略响应时，必须在读取字段前通过后续响应等途径先写入有效值。不同摘要不得依赖初始化响应顺序。

本事件不发送给普通实体，也不返回效果程序或产生执行观察点。
