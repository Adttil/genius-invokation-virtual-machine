[开发备忘](../notes.md) / **固定程序、重入与表达能力**

# 固定程序、重入与表达能力

定义源集与游戏流程在对局开始前编译为固定规则。运行时响应读取状态、修改当前事件、计算选定程序的全部输入，再交给执行器；不会临时追加规则代码。公开接口见[命令](../reference/definition/commands.md)、[响应上下文](../reference/executor/handle_context.md)与[编译](../reference/executor/compile.md)。

## 基本约束

- definition 提供核心命令集合，拓展者组合和配置命令，不注册核心以外的命令类型。
- table 保存持久游戏状态；executor 保存执行位置、事件、输入、游标等临时结算状态。
- command 所需输入由编译时的具体命令值决定，不读取外层响应的事件或响应者。
- 同一操作的固定参数和消费输入方式由同一命令类型表达；固定参数方式不消费输入，消费输入方式取得自己的专用输入帧，完成后消费输入并清除临时工作帧。
- 每个 command 不改变后续 command 的输入。终局会逻辑废弃旧结算帧，不要求逐层清理。
- 相邻 command 的输入按执行次序提交、按消费需要反序放置，因此命令序列与输入序列可以对应拼接。

## 程序入口与输入

`program_entry` 不以事件 Context 为模板参数。程序所需输入由其中动态命令按执行顺序组成，每个动态命令通过 `input_type` 指定输入类型，固定模式不占输入位置。输入对象数量、类型和顺序固定，帧内数组长度可变。debug 编译为各程序追加输入标记序列，入口保存该序列的起点和数量；invoke 写入前逐项核对数量、类型与顺序，不符时抛出 `std::invalid_argument`。Release 移除标记序列、入口中的检查字段和检查代码。

输入标记的选取仅用于 debug 检查，不决定编译出的操作。命令的编译重载按具体命令值选择固定参数或消费输入的执行路径。每个动态命令只有一个标记，即使其输入含有多个数组和固定部分。

实际字节长度仍用于运行时分配与费用缓存。公开动态适配器通过 `span<const any_command_input>` 提交输入对象序列，数组以对象中的 span 表达；适配器不暴露原始字节协议。类型标记由 C++ 包装实现提供，Lua 脚本在两种构建模式下可以保持相同写法。

初始化和回合根流程没有响应提供输入，其中的命令值必须选择固定模式。debug 编译检查这两段根流程不含动态输入标记，否则抛出 `std::invalid_argument`。响应程序正常完成后返回发起它的结算；期间终局则不再返回。无后续效果的响应返回空入口，不调用 `invoke`。

## Handler 与调用

handler 读取编译后的 definition、自身实体 view、事件、只读 table 和随机源，可以先修改事件，再一次性计算程序输入。调用方预先准备返回位置；普通响应以 `return context.invoke(entry, inputs...);` 提交时，调用对象仅写入输入并返回入口；命令执行期间不再为取得每一步参数重新回调定义源。

可变参数调用先按值取得专用输入对象，再依逆序将各命令的独立参数帧写入。数组内容在提交时复制，源数组须在复制期间保持有效，不能借用因本次扩容而移动的执行现场。公开接口不接受任意原始字节；费用缓存中的已验证帧由执行器内部复制回放。

一次响应只提交一次并立即返回 `invoke` 的结果。此前取得的栈引用可能失效，广播推进器也必须在调用后重新定位或立即交还调度，不能继续读取旧引用。

提交的输入是响应时的快照；前序命令及其嵌套响应改变牌桌后，后序输入仍保留原值。定义源应区分需要保留的快照与需要相对现值计算的修改，并保证每条命令实际执行时的目标及输入满足前提。普通即时响应推荐先消耗已承诺使用的资源，再执行可能触发其他响应的效果；不要求所有根据 table 计算的参数都由程序首条消费。费用缓存的延迟执行前提另见[费用预览与提交](event_dispatch/payment_commit.md#缓存输入的快照与执行前提)。

## 费用预览

费用响应仍接收同一种 `handle_context&`，但必须以 `context.invoke(substack_t{}, entry, inputs...)` 提交；普通响应使用不带标记的重载。是否向子栈写入由重载在编译期选择，不保存模式字段。费用提交只缓存入口和整段初始输入，确认后才执行。每个候选在一个行动窗口内只允许报价一次，费用可反复读取；不保存“已报价”标记，不进行重复调用检查。

报价时所有响应读取不变的 table，前一响应只通过费用事件影响后一响应。支付效果不会反馈到本次报价。缓存及复制策略见[费用预览与提交](event_dispatch/payment_commit.md)。

## 表达能力与效率

固定程序约束的是步骤，不约束输入值。一次响应可以捕获当前生命值 X，为一个 `deal_damage_input` 提交包含多个数值相同的 `damage` 的数组；前一次伤害的计算响应改变生命值，也不会重新计算后续初始输入。`deal_damage` 已支持消费输入，抽牌的消费参数版本仍未加入。

出战状态护盾直接在 `damage_effect` 响应中减少伤害，并提交 `modify_combat_status_state_input`，通过负的 `count` 增量交给 `modify_combat_status_state` 扣层，`round_usages` 增量保持零。命令在执行时从目标当前状态扣除，并先写入再通知自身，不借用外层伤害事件或隐含响应者。

默认构造的 `set_active_character{}` 消费 `set_active_character_input`，显式提供 `target` 时使用固定目标；默认构造的 `add_attachment{}` 消费 `add_attachment_input`，显式提供 `definition` 时使用固定参数。`deal_damage` 使用非空 `damages` 作为固定描述；否则消费一个 `deal_damage_input`，其中的伤害数组长度在响应时确定，允许为空。以上选择均在编译时完成。其他动态命令同样通过 `input_type` 指定输入，字段相符的事件可显式复用为输入别名。输入声明紧邻命令放在 `definition/commands.hpp`，不再拆成独立头文件。

`summon`、`generate_combat_status`、`attach` 及对应直接添加命令携带本次 `state`，固定参数和动态输入均默认两个字段为 `UINT32_MAX`。执行时逐字段裁剪到定义通过 `*_state_limit` 查询提供的上限，缺少查询时上限同样为 `UINT32_MAX`；它是普通数值，不是省略状态的哨兵。绝对赋值同样裁剪到上限，相对修改以 `std::int64_t` 增量在执行时按当前值饱和到 `[0, 上限]`。详细生成、重复生成、耗尽与通知边界见[实体事件](event_dispatch/entity_events.md)。

固定版本不能机械地编译成“压入输入的 opcode，再跳到消费版本”。通常固定版与消费版各有开头执行函数，将初始化和首个连续不可中断步骤一起完成，后续恢复点才复用；避免固定版增加一次调度。

## 一次完整执行与一次完整结算

每次 execute 调用都同步运行至返回，不保留尚未返回的 C++ 调用。需要继续的工作由执行位置和栈上的运行数据表达：广播可以返回同一执行位置继续游标，付款响应返回同一支付循环入口，从已经推进的游标继续下一项，条件分支和循环直接改变执行位置。

不再在栈尾保存 `stage_t`，也不在取到 execute 后再次按 stage 分派。恢复点由执行位置直接选择函数。事件、输入和循环游标仍然是运行数据，不能因为移除 stage 就消除这些数据。

完整 command 在进入下一个 command 前须消费自己的输入并清除临时帧，保留后续输入和返回记录。固定参数版本不消费输入。正常返回的响应程序保留调用者的结算现场；终局是明确的逻辑废弃例外。

## 程序的内部连接

核对位置：[library.hpp](../../../include/givm/executor/library.hpp)、[start_round.hpp](../../../include/givm/executor/commands/start_round.hpp) 与 [executor.hpp](../../../include/givm/executor/executor.hpp)。

执行位置、入口、返回位置和跳转目标统一为字节偏移，执行位置指向当前 execute 函数指针。空入口使用内部哨兵；其数值以及根入口位置均不公开。编译和执行不保存指向 vector 元素的长期地址，因此扩容和定义库移动不会改变已记录的偏移。

初始化段结束处自动添加首次回合推进；普通模式的回合末尾将递增回合数、判限、清骰和回跳合在一个 opcode 中，直接回到回合命令序列。观察模式在递增后暂停，首轮与后续回合共用判限和清骰的恢复点，不会重复递增。`start_round` 仅广播规则通知，由回合命令序列显式安排在投骰之后。

根编译用两个内部编译值标记首次进入和回合末尾，沿用 `append_commands` 的命令编译机制；它们不属于公开命令集合。相关 opcode 及编译重载完整定义在 `commands/start_round.hpp`，根编译只提供位置，不解释回合指令布局。通用 executor 不包含回合规则，也不依靠分开的函数声明和定义来绕过头文件依赖。

每段响应程序末尾追加返回。调用方先保存外层本方和恢复位置，在返回记录之上反序放置全部输入；进入响应程序时将 `table.state().self_player` 设为响应实体所属玩家。响应程序逐项消费输入，末尾返回函数恢复外层本方并跳转，不弹出返回记录。一轮广播或支付循环复用同一返回记录，由调用方结束该流程时清理；内层广播另行准备自己的记录。handler 调用本身与费用预览均不切换本方。费用缓存提交时只复制当前要执行的一段输入，不反转整个候选缓存。

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
