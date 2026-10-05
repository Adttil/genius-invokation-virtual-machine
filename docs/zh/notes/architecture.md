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

`definition_library` 可以被多局游戏共享，它不是游戏状态。在给定定义库下，一局游戏的可变状态由 `table` 与 `executor` 共同组成。table 只保存游戏状态与定义 ID，不持有 definition library。executor 也不保存库指针；每次视图提交或 `resume` 显式接收与程序现场和实体定义 ID 配套的定义库。

牌组等每局输入不编入游戏规则程序。上层在对局开始前用 `issued_id_map` 链接名称，再由 `load_deck` 把 `linked_deck` 装入 table 并完成角色状态与技能初始化；随机洗牌、抽牌和出战角色选择等规则步骤由游戏流程命令执行。具体接口见 [牌组链接与装载](deck_initialization.md)。

临时输入与调用现场保存在主执行栈中，待处理记录分别保存在伤害、入手和混合记录栈中。独立结算域的头部与其混合记录放在同一帧，根流程使用同样的布局；通用执行上下文不另存根范围或当前域定位。具体归属见[响应返回、分段与延迟程序](settlement_protocol.md)。随机源由每次执行时传入的随机函数提供，核心不持有生成器；已经取得的预发随机值可以保存在栈中，随 executor 一起复制。

## 游戏规则程序

定义源集（包括每张卡牌、角色和状态的效果定义）与游戏流程规则共同编译为一个游戏规则程序。executor 运行时保存该程序中的当前执行位置，并从这里继续推进游戏。

游戏流程规则由只执行一次的初始化程序和反复执行的回合程序描述。卡牌、角色等定义对事件的响应可以根据当前情况修改事件，或者选择规则程序中已经存在的响应子程序。运行期间不会临时生成新的规则步骤。

定义源和游戏流程只能使用核心提供的公开命令集。这一直是有效的接口约定；编译入口按核心 command 集合检查，不提供自定义指令注册接口。

各段流程在编译结果中如何连接不属于公开接口。程序内部的跳转和返回由执行器收束，不单独交给外层驱动。公开命令用于描述规则程序；上层运行对局时通过执行现场观察领域结果，不再取得当前指令或执行位置。

固定程序、程序入口、Context 与表达能力边界见 [固定程序模型](fixed_program.md)。

## 命令与事件

命令是定义源和游戏流程使用的规则描述，保存预先确定的操作参数；执行期间使用的输入、事件现场和游标进入主执行栈，待处理通知进入所属域的记录栈。恢复点由执行位置表示，对牌桌的持久修改进入 table。完整内部指令执行与公开观察边界分别组织，原因见[执行观察与输入](execution_observation.md)。

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

- `definition.hpp`：定义源协议、源库及其视图、ID 映射、公开 command 与命令 variant、事件、程序入口，以及牌组名称链接。
- `definition_source_interface.hpp`：definition 模块的源库公开入口，提供完整的源库登记、持有、合并、查询、基础定义配置和源准备诊断能力；不包含命令集合或执行器。
- `definition_common.hpp`：definition 模块的较窄聚合头，保留完整 source view、定义运行数据、事件、查询和命令描述，不包含源库容器；供 executor 按模块边界使用。
- `table.hpp`：牌桌状态、`issued_id` 及其 `definition_id`、`tag_id` 别名、定义类别、实体 ID、实体访问对象、`linked_deck` 和 `table`。
- `executor.hpp`：最终编译、编译上下文、编译后的定义库、随机输入和 `executor`。
- `utils/stack.hpp`：可独立使用的栈与 frame view 工具；其公开性不意味着 executor 提供原始栈访问。

面向使用场景的聚合头还包括 `definition_source.hpp`、`compile.hpp` 和 `runtime.hpp`：分别提供定义源编写、整库编译和对局运行能力，`givm.hpp` 汇总全部公开能力及官方基础定义。这些入口可以组合，不必形成线性包含层级；`definition.hpp`、`executor.hpp` 继续保留原有完整模块接口。`definition_source.hpp` 与 `compile.hpp` 不包含 executor 或输入/观察视图；`runtime.hpp` 不包含源库容器和完整编译上下文，但保留完整 source view、公开模板及内联报价需要的命令类型和响应上下文。

跨核心模块通过指定的公共聚合头，依赖方向保持一致。executor 的运行部分使用 `definition_common.hpp`，需要源库的编译部分使用 `definition_source_interface.hpp` 或完整 `definition.hpp`，不直接包含 definition 的叶文件。definition 使用 `table.hpp` 提供的游戏数据类型，不包含 executor 实现。table 的直接及传递包含均不进入 definition 或 executor；executor 通过 source view 完成最终编译。需要提及上层类型时使用适当的前置声明；前置声明本身不把类型定义的归属搬到下层。

命令拆分为文件后仍遵守这一边界：definition 的命令文件通过 `table.hpp` 使用牌桌类型，`src/executor/commands` 中的命令实现通过 definition 模块的聚合头使用公开命令和错误类型，不穿过模块聚合头直接包含另一模块的叶文件。移动到 `src` 不改变模块归属或依赖方向；同模块的私有头仍可以直接包含有实际复用关系的其他文件。[`definition/commands.hpp`](../../../include/givm/definition/commands.hpp) 与 [`src/executor/commands.hpp`](../../../src/executor/commands.hpp) 均只汇总包含；命令错误的类型和单项格式化留在 definition，编译总错误的位置和格式化留在 executor，不形成反向依赖。

命令的 `check`、编译函数、opcode、调试输入标记生成和广播实现仅供后端使用，声明也不向用户交付。公开 `compile` 收集并返回命令的结构化错误。视图需要的类型和内联工具直接放在各自视图头中，命令实现可以包含本模块视图头，公开视图不再包含私有命令头。

table 中的 `issued_id` 通过 `friend class issued_id_map;` 直接授予 definition 中的 ID 映射类友元权限，由后者发行有效 ID。友元声明不要求另行前置声明该类或包含上层模块头文件，不改变包含依赖方向。

definition 中的 source 适配只传递 `definition_compile_context&`，因此可以使用前置声明。`program_entry` 的完整类型归 definition，保存程序入口，不绑定外层事件或响应者；索引的生成与解释、完整编译上下文及编译执行实现仍归 executor。公开 command、`any_command` variant 与事件同样归 definition。定义拓展者通过 `definition_source.hpp` 取得命令、事件、只读定义库和完整的编译、响应上下文，而无需引入 executor 和各个视图。source 适配仅传递 `handle_context<TEntity>&`，因此可以前置声明这个类模板，保持单向依赖。

源库公开登记、名称查找和按类别遍历 source view 的能力。选择、依赖闭包和 ID 分配仍由源库已有私有准备算法完成，`definition_library` 通过原有友元关系访问；不再公开独立的 ID 映射构建入口。executor 中的非成员 `compile(sources, ..., initialization_program, round_program, mode)` 返回完整定义库及同次编译的 `id_map`。`definition_selection` 放在 `executor/compile.hpp`，属于整库编译接口；`source.compile(context)` 仍是单项定义源协议。

源库的公开模板保留在头文件中，合并冲突检查、依赖解析、定义选择、基础定义补入、ID 映射构建及格式化等较重实现位于 `src/definition/source_library.cpp`。有限定义类别的算法可以在 cpp 中显式实例化，调用方不重复实例化其实现。ID 映射的标签筛选实现位于 `src/definition/issued_id_map.cpp`。`source_view` 的源码模板和原有两指针表示保持不变；源库仍依赖源协议中的 table、事件和查询类型，本阶段不引入独立的注册接口或拆分 table。

`definition_library` 的公开部分及命令序列包装工具位于 [`executor/library.hpp`](../../../include/givm/executor/library.hpp)，完整编译上下文位于 [`executor/definition_compile_context.hpp`](../../../include/givm/executor/definition_compile_context.hpp)，整库编译入口与结果位于 [`executor/compile.hpp`](../../../include/givm/executor/compile.hpp)，非模板程序编译后端在 `src` 中实现。`source_view` 保持完整，不单独拆出其函数指针类型；源诊断的共有类型独立放在 `definition/source_error.hpp`，让运行期异常可以使用 `definition_name` 而无需包含源库容器。命令序列先统一为 `span<const any_command>`，因此定义源编译单元不必实例化命令编译循环。内部类型按功能放在所属模块的文件中，可继续使用 `givm::detail` 命名空间，不另建 `detail/` 目录；较大的内部实现块直接写成 `namespace givm::detail`，保持单层命名空间缩进。

仍在头文件中实现的函数，在定义处完整给出，不另写重复签名的声明；自由函数须为 `inline` 或 `constexpr`。移入 cpp 的函数使用通常的头文件声明与 cpp 定义。仅实现需要共享的声明和内联工具放在 `src` 对应目录的私有头中，公开头不包含这些私有头。禁止通过重复友元函数声明或头文件拼接顺序掩盖循环依赖。完整迁移计划见[头文件分层与编译后端迁移](header_layers.md)。

终局由 definition 中的 `end_game` command 表达并由 executor 执行，构建程序时不预装三条终局指令。公开命令是规则描述，最终编译产物的表示由 executor 决定，两者不必保持一一对应。程序的具体连接仍不构成公开接口，详见[固定程序记录](fixed_program.md#程序的内部连接)。

[返回文档入口](../notes.md)

## 维护时需要保留的区别

- 定义库、牌组链接信息、牌桌和执行器分别承担规则、对局前名称解析、持久局面和临时结算，不应把随机生成器、界面展示或每局牌组重新塞入编译规则。
- table 是独立的游戏数据模块，definition 使用它的定义 ID 和实体类型。最终编译及编译后的 definition library 归 executor，源库继续归 definition。
- 复制牌桌和执行器并不自动复制外部随机源、输入日志或界面任务。每一部分都需要由持有者明确选择复制、共享或重建。
- 此处保留的持久化设想是“保存足以重建定义库的信息”，没有承诺直接序列化运行时指针、type index 或栈字节。
