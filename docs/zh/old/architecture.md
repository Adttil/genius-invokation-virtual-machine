# 架构设计总览

核心库将游戏规则与一局游戏的状态分开建模。规则在对局开始前由定义源和游戏流程共同构成；对局运行时，执行者按照这些规则推进游戏并修改状态。

核心只负责确定性的规则执行和公开观察协议。界面文字、动画编排、候选项展示、输入合法性判断和日志格式由上层实现。

## 核心对象

- `definition_source_library` 收集卡牌、角色、状态等分类定义及其依赖。
- `definition_library` 是定义源集与游戏流程规则共同编译得到的不可变游戏规则。
- `issued_id_map` 是同次编译产生的对局前链接信息，用于把名称形式的输入转换为 issued ID。
- `linked_deck` 是已经链接到相应定义库的牌组，只保存卡牌和角色的 issued ID。
- `card_table` 保存一局游戏中持续存在的牌桌状态，例如实体、资源和回合信息。
- `executor` 保存当前执行位置和结算过程中的临时状态，并负责推进规则。

`definition_library` 可以被多局游戏共享，它不是游戏状态。在给定定义库下，一局游戏的可变状态由 `card_table` 与 `executor` 共同组成。table 引用相应的 definition library，因此该定义库必须在 table 的整个生命周期内保持有效。

牌组等每局输入不编入游戏规则程序。上层在对局开始前用 `issued_id_map` 链接名称，并把 `linked_deck` 装入 table；随机洗牌、角色初始化等规则步骤由游戏流程指令执行。具体接口见 [牌组链接与装载](deck.md)。

临时事件、输入槽和尚未完成的结算保存在 executor stack 中。随机源由每次执行时传入的随机函数提供，核心不持有生成器；已经取得的预发随机值可以保存在 stack 中，随 executor 一起复制。

## 游戏规则程序

定义源集（包括每张卡牌、角色和状态的效果定义）与游戏流程规则共同编译为一个游戏规则程序。executor 运行时保存该程序中的当前执行位置，并从这里继续推进游戏。

游戏流程规则由只执行一次的初始化程序和反复执行的回合程序描述。卡牌、角色等定义对事件的响应可以根据当前情况修改事件，或者选择规则程序中已经存在的响应子程序。运行期间不会临时生成新的规则步骤。

定义源和游戏流程只能使用核心提供的公开指令集。各段流程在编译结果中如何连接不属于公开接口；对局运行期间，上层观察到的当前指令始终来自公开指令集。

固定程序、程序入口、Context 与表达能力边界见 [固定程序模型](fixed-program-design.md)。

## 指令与事件

指令是上层可以识别和观察的规则步骤。指令保存预先确定的操作参数；执行期间产生的阶段和临时数据进入 executor stack，对牌桌的持久修改进入 table。

event 描述一次正在结算、允许响应者修改的规则事件。handler 可以读取自身定义、实体、只读 table 和随机输入，并修改 event；需要产生后续效果时，handler 从已经编译的响应程序中选择入口。发起事件的指令负责完成广播、应用最终事件结果和清理本次临时状态。

每个公开指令和事件的字段、执行顺序、挂起点与栈 ABI 都是接口契约，见 [当前指令与事件目录](events.md)。通用响应协议见 [指令、栈与事件系统](event-system.md)。

## 上层驱动与观察

上层先以 `executor.enter_entry(table.definition_library())` 进入游戏主流程，再通过 `executor::execute_next(table, random)` 推进规则。上层通过 `executor.status()` 判断对局结果，并在需要输入、日志或动画时结合 `executor.position()`、`table.definition_library().instruction(executor.position())`、table 和 executor stack 观察当前结算状态。

需要输入的指令会按照自身文档规定的栈 ABI 留下输入位置并暂停。上层识别当前指令，读取 table 与 stack 生成合法候选项，再写入完整输入对象并继续执行。核心不保存界面候选项，也不替上层验证交互来源是否合法。

详细的驱动、取指和生命周期约定见 [Table、Executor 与外层观察](table-vm-and-input.md)；stack 访问规则见 [Executor Stack](stack.md)；随机记录与回放见 [随机输入](random-input.md)。

## 复制与清理

分支模拟复制匹配的 table 与 executor，并继续引用同一份不可变 definition library。持久化时需要同时记录足以重建所用定义库的构建信息。

实体离场通常先标记为无效，`table.clean_up()` 才会压缩存储。调用者必须确认活动结算、日志和展示任务不再引用会失效的实体位置。具体清理边界见 [Table、Executor 与外层观察](table-vm-and-input.md)。

## 模块入口

三个核心模块的依赖方向为：

```text
definition -> table -> executor
```

- `definition.hpp`：定义源、编译定义库、issued id、程序入口，以及牌组名称链接与 `linked_deck`。
- `table.hpp`：牌桌状态、实体 ID、实体访问对象和 `card_table`。
- `executor.hpp`：公开指令、事件、随机输入和 `executor`。
- `utils/stack.hpp`：执行栈与 frame view。

跨模块代码使用这些外部入口头。模块内部的组织方式不构成公开接口。

[返回文档入口](README.md)
