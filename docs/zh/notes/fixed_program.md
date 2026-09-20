[开发备忘](../notes.md) / **固定程序、重入与表达能力**

# 固定程序、重入与表达能力

定义源集与游戏流程在对局开始前编译为固定规则。运行时响应读取状态、修改当前事件、计算选定程序的全部输入，再交给执行器；不会临时追加规则代码。公开接口见[命令](../reference/definition/commands.md)、[响应上下文](../reference/executor/handle_context.md)与[编译](../reference/executor/compile.md)。

## 基本约束

- definition 提供核心命令集合，拓展者组合和配置命令，不注册核心以外的命令类型。
- table 保存持久游戏状态；executor 保存执行位置、事件、输入、游标等临时结算状态。
- command 所需输入由编译时的具体命令值决定，不读取外层响应的事件或响应者。
- 同一操作的固定参数和消费输入方式由同一命令类型表达；固定参数方式不消费输入，消费输入方式取得自己的初始事件，完成后消费输入并清除临时工作帧。
- 每个 command 不改变后续 command 的输入。终局会逻辑废弃旧结算帧，不要求逐层清理。
- 相邻 command 的输入按执行次序提交、按消费需要反序放置，因此命令序列与输入序列可以对应拼接。

## 程序入口与输入

`program_entry` 不再以事件 Context 为模板参数。程序所需输入由其中实际消费输入的命令按执行顺序组成，是否消费输入由各命令值决定。每个已编译程序所需输入的数量、类型和顺序固定，响应时可以选择入口并计算对应的输入值；调用者保证输入匹配，违反协议属于未定义行为。debug 构建只为每个入口保存所需输入的总字节数，逐项输入与整段字节提交都在写入前比较总长，不符时抛出 `std::invalid_argument`。不保存输入类型表，不要求动态源提供类型元信息，也不能识别同长度输入的错误类型或顺序。Release 不保存这项检查数据，也不执行检查。

`detail::input_size(command)` 仅在 debug 构建存在，用于按具体命令值累计上述检查信息；它不是公开命令协议，也不决定编译出的操作。各命令的编译重载自行按命令值选择固定参数或消费输入的执行路径。

输入范围的实际字节长度用于分配、缓存与执行，是必要运行数据。动态源可以只提交完整参数段的字节 span，不额外提供类型或逐项描述符。

初始化和回合根流程没有响应提供输入，其中的命令值必须选择不消费响应输入的方式。编译期间，debug 构建累计命令所需输入的总字节数，并检查这两段根流程的总长为零；非零则抛出 `std::invalid_argument`。响应程序正常完成后返回发起它的结算；期间终局则不再返回。无后续效果的响应返回空入口，不调用 `invoke`。

## Handler 与调用

handler 读取编译后的 definition、自身实体 view、事件、只读 table 和随机源，可以先修改事件，再一次性计算程序输入。调用方预先准备返回位置；普通响应以 `return context.invoke(entry, events...);` 提交时，调用对象仅写入输入并返回入口；命令执行期间不再为取得每一步参数重新回调定义源。

可变参数调用先按值取得初始事件，再扩容写入。整段字节调用要求源字节在整个调用期间有效，不能指向可能因本次调用而失效的现场；目标输入段由执行器复制保存。输入中进一步借用的数据仍须由定义源保证生命周期，尤其不能把临时事件引用放进延迟支付效果。

一次响应只提交一次并立即返回 `invoke` 的结果。此前取得的栈引用可能失效，广播推进器也必须在调用后重新定位或立即交还调度，不能继续读取旧引用。

## 费用预览

费用响应仍接收同一种 `handle_context&`，但必须以 `context.invoke(substack_t{}, entry, events...)` 提交；普通响应使用不带标记的重载。是否向子栈写入由重载在编译期选择，不保存模式字段。费用提交只缓存入口和整段初始输入，确认后才执行。每个候选在一个行动窗口内只允许报价一次，费用可反复读取；不保存“已报价”标记，不进行重复调用检查。

报价时所有响应读取不变的 table，前一响应只通过费用事件影响后一响应。支付效果不会反馈到本次报价。缓存及复制策略见[费用预览与提交](event_dispatch/payment_commit.md)。

## 表达能力与效率

固定程序约束的是步骤，不约束输入值。后续接入抽牌与伤害的消费参数版本后，一次响应可以捕获当前生命值 X，同时准备抽 X 张和造成 X 点伤害的初始事件；前一操作的嵌套响应不会重新计算后一操作的输入。本轮尚未加入这两个消费参数版本。

出战状态护盾直接在 `damage_effect` 响应中减少伤害，并提交 `combat_status_count_reduction` 交给 `reduce_combat_status_count` 扣除指定出战状态计数。命令不再借用外层伤害事件或隐含响应者。

默认构造的 `set_active_character{}` 消费 `active_character_changed`，显式提供 `target` 时使用固定目标；默认构造的 `add_attachment{}` 消费 `attachment_addition`，显式提供 `definition`、`state` 时使用固定定义和初始状态，为指定一方执行时的出战角色添加附件。两个选择都在编译时完成。`remove_attachment` 与 `reduce_combat_status_count` 仍只支持消费输入，其他命令仍只支持固定参数。

固定版本不能机械地编译成“压入输入的 opcode，再跳到消费版本”。通常固定版与消费版各有开头执行函数，将初始化和首个连续不可中断步骤一起完成，后续恢复点才复用；避免固定版增加一次调度。

## 一次完整执行与一次完整结算

每次 execute 调用都同步运行至返回，不保留尚未返回的 C++ 调用。需要继续的工作由执行位置和栈上的运行数据表达：广播可以返回同一执行位置继续游标，付款响应返回同一支付循环入口，从已经推进的游标继续下一项，条件分支和循环直接改变执行位置。

不再在栈尾保存 `stage_t`，也不在取到 execute 后再次按 stage 分派。恢复点由执行位置直接选择函数。事件、输入和循环游标仍然是运行数据，不能因为移除 stage 就消除这些数据。

完整 command 在进入下一个 command 前须消费自己的输入并清除临时帧，保留后续输入和返回记录。固定参数版本不消费输入。正常返回的响应程序保留调用者的结算现场；终局是明确的逻辑废弃例外。

## 程序的内部连接

核对位置：[library.hpp](../../../include/givm/executor/library.hpp) 与 [executor.hpp](../../../include/givm/executor/executor.hpp)。

执行位置、入口、返回位置和跳转目标统一为字节偏移，执行位置指向当前 execute 函数指针。空入口使用内部哨兵；其数值以及根入口位置均不公开。编译和执行不保存指向 vector 元素的长期地址，因此扩容和定义库移动不会改变已记录的偏移。

初始化段自然进入回合段，回合末尾追加内部回跳。每段响应程序末尾追加返回。调用方先保存恢复位置，在返回记录之上反序放置全部输入；响应程序逐项消费输入，末尾返回函数只读取保存的位置并跳转，不弹出返回记录。一轮广播或支付循环复用同一返回位置，由调用方结束该流程时清理；内层广播另行准备自己的返回位置。费用缓存提交时只复制当前要执行的一段输入，不反转整个候选缓存。

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

每个 command 的编译重载与执行指令集中在 [`executor/commands/`](../../../include/givm/executor/commands) 的对应实现文件内，由 [`commands.hpp`](../../../include/givm/executor/commands.hpp) 统一汇总。编译函数和执行函数直接定义在 `givm::detail`，函数名称描述具体操作。ADL 通过实际的 `detail::program_writer` 类型找到 `detail::compile(writer, command, mode)`；无需 command_backend 类或全局 opcode 编号。函数直接在定义处提供实现，汇总头包含各 command 实现后，编译上下文才实例化统一遍历。

多个 command 使用的基础执行指令仍随其所属 command 放置：抽牌通知推进及相关辅助函数放在 [`draw_cards.hpp`](../../../include/givm/executor/commands/draw_cards.hpp)，[`replace_cards.hpp`](../../../include/givm/executor/commands/replace_cards.hpp) 直接包含并复用；元素反应推进及相关辅助函数放在 [`apply_element.hpp`](../../../include/givm/executor/commands/apply_element.hpp)，[`deal_damage.hpp`](../../../include/givm/executor/commands/deal_damage.hpp) 直接包含并复用。各 command 的其他专属执行函数保留在各自文件中，共用关系由这些直接依赖表达。

definition 保留 command、variant、事件与程序入口类型。入口索引的生成与解释由 executor 负责。编译上下文、定义库、整体编译入口及返回、跳转控制函数集中在 executor 的 `library.hpp` 中；各 command 的编译与执行由上述文件组织。

[返回架构总览](architecture.md)
