[开发备忘](../notes.md) / **模块边界与状态所有权**

# 模块边界与状态所有权

本页记录模块划分对实现和后续开发的约束，尤其是状态归属、对局外输入和清理责任。公开接口从[参考手册](../reference.md)查阅。

来源：提交 `b3d6c50` 中的 `docs/zh/old/architecture.md`。这里只保留模块划分、状态归属及其设计约束。

核心库将游戏规则与一局游戏的状态分开建模。规则在对局开始前由定义源和游戏流程共同构成；对局运行时，执行者按照这些规则推进游戏并修改状态。

核心只负责确定性的规则执行和公开观察协议。界面文字、动画编排、候选项展示、输入合法性判断和日志格式由上层实现。

## 核心对象

- `definition_source_library` 收集卡牌、角色、状态等分类定义及其依赖。
- `definition_library` 是定义源集与游戏流程规则共同编译得到的不可变游戏规则。
- `issued_id_map` 是同次编译产生的对局前链接信息，用于把名称形式的输入转换为 issued ID。
- `linked_deck` 是已经链接到相应定义库的牌组，只保存卡牌和角色的 issued ID。
- `table` 保存一局游戏中持续存在的牌桌状态，例如实体、资源和回合信息。
- `executor` 保存当前执行位置和结算过程中的临时状态，并负责推进规则。

`definition_library` 可以被多局游戏共享，它不是游戏状态。在给定定义库下，一局游戏的可变状态由 `table` 与 `executor` 共同组成。table 只保存游戏状态与定义 ID，不持有 definition library。executor 也不保存库指针；每次 `step` 显式接收与程序现场和实体定义 ID 配套的定义库。

牌组等每局输入不编入游戏规则程序。上层在对局开始前用 `issued_id_map` 链接名称，再由 `load_deck` 把 `linked_deck` 装入 table 并完成角色状态与技能初始化；随机洗牌、抽牌和出战角色选择等规则步骤由游戏流程命令执行。具体接口见 [牌组链接与装载](deck_initialization.md)。

临时事件、输入槽和尚未完成的结算保存在 executor stack 中。随机源由每次执行时传入的随机函数提供，核心不持有生成器；已经取得的预发随机值可以保存在 stack 中，随 executor 一起复制。

## 游戏规则程序

定义源集（包括每张卡牌、角色和状态的效果定义）与游戏流程规则共同编译为一个游戏规则程序。executor 运行时保存该程序中的当前执行位置，并从这里继续推进游戏。

游戏流程规则由只执行一次的初始化程序和反复执行的回合程序描述。卡牌、角色等定义对事件的响应可以根据当前情况修改事件，或者选择规则程序中已经存在的响应子程序。运行期间不会临时生成新的规则步骤。

定义源和游戏流程只能使用核心提供的公开命令集。这一直是有效的接口约定；编译入口按核心 command 集合检查，不提供自定义指令注册接口。

各段流程在编译结果中如何连接不属于公开接口。程序内部的跳转和返回由执行器收束，不单独交给外层驱动。公开命令用于描述规则程序；上层运行对局时通过执行现场观察领域结果，不再取得当前指令或执行位置。

固定程序、程序入口、Context 与表达能力边界见 [固定程序模型](fixed_program.md)。

## 命令与事件

命令是定义源和游戏流程使用的规则描述，保存预先确定的操作参数；执行期间产生的事件、输入和游标进入 executor stack，恢复点由执行位置表示，对牌桌的持久修改进入 table。完整内部指令执行与公开观察边界分别组织，原因见[执行观察与输入](execution_observation.md)。

event 描述一次正在结算、允许响应者修改的规则事件。handler 可以读取自身定义、实体、只读 table 和随机输入，并修改 event；需要产生后续效果时，handler 从已经编译的响应程序中选择入口。发起事件的指令负责完成广播、应用最终事件结果和清理本次临时状态。

命令描述和事件的字段均是公开接口：定义源需要直接提供命令，响应函数需要直接访问事件。指令执行函数、执行上下文、阶段和完整帧由内部实现维护。executor 不公开其栈，读取执行现场和提交输入都通过相应 execution_view；规则事件可由 handler 修改，观察现场不会把这项权限交给上层。

## 复制与清理

分支模拟复制匹配的 table 与 executor，并在继续推进时显式传入配套的不可变 definition library。持久化时需要同时记录足以重建所用定义库的构建信息。

实体离场先标记为无效，将压缩存储延后到安全点，避免结算期间的实体身份因搬迁改变。这一取舍及未采用 generation、free list 的原因见[牌桌存储设计](table_storage.md)；调用方的清理条件见 [`clean_up`](../reference/table/table/clean_up.md)。

## 模块入口

三个核心模块的包含依赖方向如下，箭头指向被依赖的模块：

```text
executor -> definition -> table
executor ----------------> table
```

- `definition.hpp`：定义源协议、源库及其视图、依赖选择、ID 映射、公开 command 与命令 variant、事件、程序入口，以及牌组名称链接。
- `table.hpp`：牌桌状态、`issued_id` 及其 `definition_id`、`tag_id` 别名、定义类别、实体 ID、实体访问对象、`linked_deck` 和 `table`。
- `executor.hpp`：最终编译、编译上下文、编译后的定义库、随机输入和 `executor`。
- `utils/stack.hpp`：可独立使用的栈与 frame view 工具；其公开性不意味着 executor 提供原始栈访问。

跨核心模块包含公共入口，依赖方向保持一致。definition 使用 table 提供的游戏数据类型，不包含 executor 实现。跨模块包含保持从上层指向下层。table 的直接及传递包含均不进入 definition 或 executor；definition 使用 table 提供的游戏数据类型，executor 通过 source view 完成最终编译。需要提及上层类型时使用适当的前置声明；前置声明本身不把类型定义的归属搬到下层。

table 中的 `issued_id` 通过 `friend class issued_id_map;` 直接授予 definition 中的 ID 映射类友元权限，由后者发行有效 ID。友元声明不要求另行前置声明该类或包含上层模块头文件，不改变包含依赖方向。

definition 中的 source 适配只传递 `definition_compile_context&`，因此可以使用前置声明。`program_entry` 的完整类型归 definition，保存程序入口，不绑定外层事件或响应者；索引的生成与解释、完整编译上下文及编译执行实现仍归 executor。公开 command、`any_command` variant 与事件同样归 definition。定义拓展者可以仅包含 `definition.hpp` 保存入口与初始事件；实际调用 `add_program` 或 `handle_context` 成员时包含 `givm.hpp`，取得完整实现。source 适配仅传递 `handle_context&`，因此同样可以前置声明，保持单向依赖。

源库提供登记、名称查找、按类别遍历 source view 和建立 ID 映射的能力，不提供成员编译函数。成员 `sources.make_issued_id_map(...)` 使用登记时保留的声明信息完成选择、依赖闭包和 ID 分配；executor 中的非成员 `compile(sources, ..., initialization_program, round_program, mode)` 调用这个成员取得映射，再通过 source view 构建完整定义库。`source.compile(context)` 仍是单项定义源协议，不与整库编译入口混淆。

`definition_library`、整库 `compile` 及相关程序装配细节集中在 [`executor/library.hpp`](../../../include/givm/executor/library.hpp)。内部类型按功能放在所属模块的文件中，可继续使用 `givm::detail` 命名空间，不另建 `detail/` 目录；较大的内部实现块直接写成 `namespace givm::detail`，保持单层命名空间缩进。函数在定义处完整给出，不另写重复签名的声明。这也适用于 `friend` 函数：不能在类内只写友元函数声明，再到类外重复签名定义。这些约束要求从类型归属、前置声明和依赖方向解决包含问题，而不是用头文件拼接顺序掩盖循环。

终局由 definition 中的 `end_game` command 表达并由 executor 执行，构建程序时不预装三条终局指令。公开命令是规则描述，最终编译产物的表示由 executor 决定，两者不必保持一一对应。程序的具体连接仍不构成公开接口，详见[固定程序记录](fixed_program.md#程序的内部连接)。

[返回文档入口](../notes.md)

## 维护时需要保留的区别

- 定义库、牌组链接信息、牌桌和执行器分别承担规则、对局前名称解析、持久局面和临时结算，不应把随机生成器、界面展示或每局牌组重新塞入编译规则。
- table 是独立的游戏数据模块，definition 使用它的定义 ID 和实体类型。最终编译及编译后的 definition library 归 executor，源库继续归 definition。
- 复制牌桌和执行器并不自动复制外部随机源、输入日志或界面任务。每一部分都需要由持有者明确选择复制、共享或重建。
- 此处保留的持久化设想是“保存足以重建定义库的信息”，没有承诺直接序列化运行时指针、type index 或栈字节。
