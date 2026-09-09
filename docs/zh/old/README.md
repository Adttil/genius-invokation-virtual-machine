# Genius Invokation Virtual Machine 文档

本目录记录核心库对上层使用者和规则拓展者公开的行为契约。文档以语义、可观察状态和栈 ABI 为中心；未被明确列为接口的内部实现可以在不改变这些契约的前提下调整。

## 文档边界

- [当前指令与事件目录](events.md) 是现行规则接口的权威索引。每个指令和事件页面记录的字段、语义、执行顺序、挂起点与栈 ABI 是实现和优化必须保持的契约。
- [Executor Stack](stack.md) 记录公开栈操作及 view 生命周期。stack 虽是低层工具，但上层需要据此观察输入、日志和动画状态，因此属于公开接口。
- 总览文档解释模块职责和跨类型协议，不重复具体指令或事件页面已经规定的布局。
- 明确标为“设计记录”或“草案”的文档用于保存讨论背景，不代表现行接口。

## 阅读入口

- [架构设计总览](architecture.md)：模块边界、状态所有权和执行模型。
- [固定程序模型](fixed-program-design.md)：固定程序、强类型入口、Context 和表达能力边界。
- [编写定义源](definition-system.md)：规则拓展接口，从身份与依赖开始介绍 definition 编译和响应程序。
- [指令、栈与事件系统](event-system.md)：指令重入、响应程序、广播和 onpay 公共协议。
- [Table、Executor 与外层观察](table-vm-and-input.md)：上层驱动、取指、输入、日志和动画观察。
- [Executor Stack](stack.md)：frame、动态数组、访问和失效规则。
- [随机输入](random-input.md)：随机函数、挂起、记录与回放。
- [当前指令与事件目录](events.md)：具体指令和事件规范。
- [最小可运行对局](game-flow.md)：一种初始化程序与回合程序组合及其可配置规则。
- [牌组链接与装载](deck.md)：按名称链接牌组、装载初始 table，以及初始化程序中的洗牌和角色初始化。
- [牌与区域实体模型](card-entity-model.md)：牌实体、区域身份与 ID 语义。
- [Table 内部数据结构设计记录](table-storage-design.md)：内部存储方案的设计记录，不是公开接口规范。

## 模块入口

`include/givm` 的主要外部入口为：

- `givm.hpp`：完整公开接口。
- `definition.hpp`：定义源、编译定义库、issued id、程序入口，以及牌组名称链接与 `linked_deck`。
- `table.hpp`：牌桌持久状态、实体 ID、实体访问对象与 `card_table`。
- `executor.hpp`：具体指令、事件、广播工具、随机输入与 `executor`。
- `utils/stack.hpp`：通用执行栈及 frame view。

三个核心模块的依赖方向为 `definition -> table -> executor`。跨模块使用外部入口头；模块内部实现可以直接包含本模块头文件。
