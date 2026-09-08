# 随机输入

随机性是每次单步执行的外部依赖，不是 executor 持有的生成器，也不是 table 中等待填写的状态。这个边界允许上层替换随机源、记录原始随机值、确定性回放和模拟分叉。

## 调用协议

`executor::execute_next` 接收一个返回 `std::uint32_t` 的可调用对象：

```cpp
executor.execute_next(table, random);
```

本次同步调用内部会把它作为非持有 `random_fn` 传给当前指令和 definition handler。核心不会把该引用写入指令、table 或 executor stack，也不会在 `execute_next` 返回后继续持有它。因此相邻两步可以使用不同的生成器、记录包装器或回放条带。

拓展者只依赖 `random_fn` 可调用并返回 `std::uint32_t` 的语义。

## 挂起与预发

指令返回 `false` 表示本次 `execute_next` 后 executor 已到达输入、观察挂起点或终局，不表示该指令在返回前不能取得随机数。

需要让玩家提交顺序不影响结果时，指令可以在挂起前预发后续随机值并保存到其公开 stack ABI 中。例如开局换牌和双方重投会先准备各自的随机池，再逐个等待输入。

上层记录器应记录所有实际发生的随机调用。即使一次调用发生在最终挂起之前，它仍属于本次同步执行步骤。

运行到挂起或终局的通用循环见 [Table、Executor 与外层观察](table-vm-and-input.md)。

## 记录与回放

核心不规定日志格式。上层可以用普通可调用对象包装真实随机源：

```cpp
struct recording_random
{
    source_type& source;
    std::vector<std::uint32_t>& tape;

    std::uint32_t operator()()
    {
        const auto value = source();
        tape.push_back(value);
        return value;
    }
};
```

回放时传入依次读取记录条带的可调用对象：

```cpp
struct tape_random
{
    std::span<const std::uint32_t> tape;
    size_t index = 0;

    std::uint32_t operator()()
    {
        return tape[index++];
    }
};
```

完整回放还需要保存定义库构建信息、初始 table，以及上层在各挂起点写入的输入。定义库构建信息用于重建匹配的不可变 definition library；定义源按类别和不透明名称标识，初始化程序和回合程序由公开指令序列描述。核心只规定随机调用顺序和数值宽度，不在本页规定这些数据的具体序列化格式。

## 模拟分叉

模拟器可以在一次 `execute_next` 返回后复制彼此匹配的 table 与 executor，并为不同分支提供不同随机源。两份 table 必须继续绑定同一份仍存活的不可变 definition library。

每个记录的 `std::uint32_t` 都是独立输入，可以单独替换。若替换导致后续控制流和随机调用次数发生变化，分支可以使用调整后的条带或在条带耗尽时切换到后备生成器。

单条指令内部是同步执行过程。只有 `execute_next` 返回后，当前执行位置、table 和 stack 才共同构成可复制、可观察的暂停状态。

## 数值语义

每次随机调用返回一个原始 `std::uint32_t`。核心不统一规定取模、拒绝采样或分布转换；具体指令文档负责说明如何使用随机值。无论该值最终是否改变结果，只要随机函数被调用，上层记录器就应把它计入条带。

[返回文档入口](README.md)
