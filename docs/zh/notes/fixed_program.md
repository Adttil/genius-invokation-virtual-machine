[开发备忘](../notes.md) / **固定程序、重入与表达能力**

# 固定程序、重入与表达能力

定义源集与游戏流程在对局开始前编译为固定规则。运行时可以读取状态、修改事件、选择分支和循环执行领域操作，但不会临时生成新的规则程序。公开接口见[命令](../reference/definition/commands.md)与[编译](../reference/executor/compile.md)；本页记录内部实现约束。

## 基本约束

- definition 提供核心命令集合。拓展者可以组合和配置命令，不能注册核心以外的命令类型。
- table 保存持久游戏状态；executor 保存执行位置、事件、输入、游标等临时结算状态。
- handler 只能选择已经编译的响应程序，不能在对局中追加程序。
- 一个 command 编译得到的整段代码正常完成时，必须恢复进入该 command 前的栈形状。借用的调用 Context 可以按规则修改；相邻 command 之间不能增加隐含的生产、消费栈协议。
- 同一 command 内部的多条指令可以通过临时帧合作。进入终局后，旧结算帧被逻辑废弃，不要求逐层清理。

公开 command 是规则描述，不规定最终编译产物由什么组成。内部指令数量、数据布局及其与 command 的对应关系均不属于 reference 的语义。

## 程序入口与 Context

`program_entry<Context>` 表示已编译响应程序的入口。Context 规定该响应可使用的事件语境，内部再将其映射到调用帧协议；不同 Context 的入口不能互换。通常使用事件类型，费用提交则使用 `onpay_context<CostEvent>`。定义作者可以通过 `handler_program_context_t<Event>` 和 `handler_program_entry_t<Event>` 使用这套映射。

空入口表示没有响应子程序；handler 已对 event 进行的修改仍然保留。终局由 `end_game{ .result = ... }` 命令表达，可用于任意相容的 Context；入口本身没有独立的终局种类。

初始化流程只执行一次，随后反复执行回合流程。响应程序正常完成后返回发起它的结算；若期间发生终局，则不再返回。初始化与回合序列要求命令与 `void` Context 兼容。

## 命令与 Context

具体命令的 `context_type = void` 表示不读取调用者 Context，可用于任意响应。依赖事件 `E` 的命令只能用于相容的 `E` 程序段。

`any_command_for<Context>` 直接是筛选后的 `std::variant` 别名，候选类型只包含兼容的核心命令，没有包装层或执行函数指针。它使用标准 variant 的构造和访问接口。编译入口接受异构 tuple-like、具体命令 range 以及 variant range，顺序消费一次；variant 通过 `std::visit` 分派到实际命令的编译重载。

`command_compatible_with` 对具体类型检查 context，对 variant 检查所有候选类型。核心集合检查与编译发生在 executor；context 兼容性不负责证明一段 execute 正确维护了栈。

旧擦除包装对整个嵌套包装再次进行 64 字节检查的问题已经消失。不同 context 的 variant 不需要先转装：只要全部候选与目标 Context 兼容，即可由 `add_program<Context>` 逐项消费。

## Handler 与响应

handler 读取 definition data、自身 entity view、只读 table 和随机输入，直接修改当前 event。持久 table 修改由返回的命令程序完成。handler 不需要查询定义库；实体的定义 ID 足以与编译时解析的依赖 ID 比较。

费用预览只保存报价与 onpay 入口。确认行动后执行所选行的 onpay，随后才扣除骰子并广播资源变化。onpay Context 保存原响应实体及该次 `cost_effect_argument`，不借用会继续变化的报价工作区。详细约束见[费用预览与提交](event_dispatch/payment_commit.md)。

## 表达能力

固定程序限制运行时生成新规则步骤，不限制动态计算。伤害随层数变化、费用调整等可在 handler 内计算；循环次数、消耗量和目标数量由领域 command 的执行过程处理。只有分支集合明确且较小时，才需要为不同分支预先登记多个响应入口，不应为无界运行时数值枚举程序。

例如层数护盾由 `absorb_damage_by_count` 在 `damage_effect` Context 中计算消耗并修改事件。它不是跟在 `deal_damage` 后消费残留栈数据：伤害广播进入另一段响应程序，抵挡命令借用的是仍然有效的调用 Context；伤害 command 完全结束后，该 Context 已清理。

command 的领域粒度与观察边界独立。一个 command 可以产生多种观察现场，也可以在普通模式下完成同一规则而不交付这些观察。

## 一次完整执行与一次完整结算

每次 execute 调用都同步运行至返回，不保留尚未返回的 C++ 调用。需要继续的工作由执行位置和栈上的运行数据表达：广播可以返回同一执行位置继续游标，付款响应返回清理入口，条件分支和循环直接改变执行位置。

不再在栈尾保存 `stage_t`，也不在取到 execute 后再次按 stage 分派。恢复点由执行位置直接选择函数。事件、输入和循环游标仍然是运行数据，不能因为移除 stage 就消除这些数据。

干净退出约束作用于完整 command。其内部指令可以保留帧给后续指令使用，但到下一个公开 command 前必须完成清理。正常返回的响应程序也必须保留调用者 Context；终局是明确的逻辑废弃例外。

## 程序的内部连接

核对位置：[library.hpp](../../../include/givm/executor/library.hpp) 与 [executor.hpp](../../../include/givm/executor/executor.hpp)。

执行位置、入口、返回位置和跳转目标统一为字节偏移，执行位置指向当前 execute 函数指针。空入口使用内部哨兵；其数值以及根入口位置均不公开。编译和执行不保存指向 vector 元素的长期地址，因此扩容和定义库移动不会改变已记录的偏移。

初始化段自然进入回合段，回合末尾追加内部回跳。每段响应程序末尾追加返回。进入响应前，调用者先设置希望恢复的执行位置，再由 `enter(entry)` 将当前执行位置压入 `frame<return_info>`。广播可以保存自身执行位置，付款流程则保存响应结束后的清理入口；返回执行函数只恢复该位置，不擅自递增。

execute 自行设置后继执行位置，调度器不会统一提前递增。普通无数据指令可以前进一个函数指针槽；带数据的指令必须跨过自身数据。终局只压入 `game_result` 并返回 `finished`，不再沿旧调用链执行。

## 指令存储的实现约束

[instruction.hpp](../../../include/givm/executor/instruction.hpp) 提供借用字节 vector 的 `detail::program_writer`，并集中定义执行函数指针类型、编译模式及字节存储的对齐规则，没有独立的拥有型 program 类。编译结果直接保存 execute 函数指针与就地数据，不保存 opcode 编号、函数表索引或每条 command 的 RTTI。

每次 `write(value)` 从当前已对齐位置写入完整对象，再把末尾补齐到 `alignof(execute_fn)`。函数指针连续写入正好相接；数据结构体整体写入，保留其内部布局。字节缓冲区也满足这个对齐，数据类型的对齐不能超过它。

一个 command 可以多次写入，并包含多组：

```text
[execute A][execute B][共享数据][execute C][仅 C 使用的数据][execute D]
```

不需要参数的后续指令可以作为独立函数复用，其栈协议由调用它的编译布局保证。共享数据的那组指令负责越过自己的数据区；不建立独立数据池或额外的操作数指针。

`context.instruction_data<N, T>(library)` 从 `context.position() + N * sizeof(execute_fn)` 取得 `const T&`，`N = 0` 也用于读取当前 execute 指针。该操作由内部 `execution_context` 提供，`definition_library` 将 `execution_context` 设为友元；读取直接使用上下文已有的执行位置，不额外接受位置参数，也不通过自由函数公开字节存储访问。读取端不复制参数。写入的数据要求平凡可复制并满足隐式生命周期条件；对齐和对象生命周期由存储实现集中负责，编译发布以及库复制完成时统一建立可读取的数据对象，而不是在每次取指时处理。

这里只记录进程内的执行表示，不定义可序列化的二进制格式。

## 编译模式与文件组织

整库 `compile` 显式接收 `compile_mode::normal` 或 `compile_mode::observed`。编译上下文保存该选择，所有 `add_program` 使用同一模式。两种模式生成相同的 `definition_library` 类型；内部指令类型、数量、数据类型与布局都可以不同。运行时统一通过 `executor::step` 推进，不再选择另一套分派入口。

每个 command 的编译重载与 execute 定义集中在其 executor 实现文件内。ADL 通过实际的 `detail::program_writer` 类型找到 `detail::compile(writer, command, mode)`；无需 command_backend 类或全局 opcode 编号。函数直接在定义处提供实现，汇总头包含各 command 实现后，编译上下文才实例化统一遍历。

definition 保留 command、variant 与事件声明。编译上下文、定义库、整体编译入口及返回、跳转控制函数集中在 executor 的 `library.hpp` 中；各 command 的编译与执行仍集中在对应实现文件内，由 `instructions.hpp` 汇总。

[返回架构总览](architecture.md)
