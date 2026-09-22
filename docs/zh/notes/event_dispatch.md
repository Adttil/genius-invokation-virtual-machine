[开发备忘](../notes.md)

# 事件分派与历史协议

这里记录广播如何恢复执行、为什么费用预览不能立即执行提交副作用，以及旧事件设计还有哪些领域流程没有实现。公开接口的用法与字段见[事件 reference](../reference/definition/events.md)。

事件 context 表达响应程序的语义限制；onpay 是现有费用事件的类型映射与提交现场，不另建事件类别。完整栈 ABI 的历史边界统一见[设计演变](history.md)。

## 运行与广播

| | |
| --- | --- |
| [指令、重入与调用现场](event_dispatch/execution_protocol.md) | const 操作数、完整 command、干净退出、执行位置恢复、activation 和 view 生命周期 |
| [默认广播与事件事务](event_dispatch/broadcast.md) | handler ID 类型、响应者快照、默认遍历顺序、游标、自身 ID、失效过滤及单目标初始化 |
| [费用预览与提交](event_dispatch/payment_commit.md) | 候选与 handler 矩阵、参数子栈、单次报价、延迟提交、取消与缓存生命周期 |

## 事件差异与历史要求

| | |
| --- | --- |
| [卡牌、技能与调和](event_dispatch/card_events.md) | 批次抽牌、逐张快照、历史 ID、效果取消与支付不回滚 |
| [资源与旧 payment](event_dispatch/resource_events.md) | 正负增量、限幅、支付后事件以及尚未接入的槽位设计 |
| [伤害、元素与治疗](event_dispatch/damage_and_element_events.md) | 属性与数值阶段、替代标签、逐段生成、组末超载和扣血后判胜 |
| [初始化、击倒与离场](event_dispatch/entity_events.md) | 局部初始化工作区、延迟出战选择意图、历史身份与完整计数器 |

## const 成员的编译器适配

当前 [`events.hpp`](../../../include/givm/definition/events.hpp) 用 `GIVM_CLANG22_TRIVIALLY_COPYABLE_WORKAROUND` 为含 const 成员的多种事件显式删除赋值运算符。这是源码为平凡复制相关编译器问题保留的处理，不意味着事件整体可以被随意赋值，也不是允许去掉 const 元数据的理由。该文件还对 `after_elemental_reaction` 的大小保留 `<= 64` 的静态断言；栈与指令存储限制分别见相应实现备忘。

## 核对范围

旧目录曾把未列出的机制一概视为草案；现有公开事件类型不能再按旧目录是否收录来判断有效性。类型已经公开，也不等于发起它的领域流程已经完成。未实现的领域流程、默认反应 TODO 和旧 payment 槽位方案在相应专题记录。

旧回合方案有两项合并记录：初始设置、规则强制设置和主动切人共用 `active_character_changed`，不再另设 switched 通知；初始投骰由准备事件收集修改后直接生成结果，不另设初始投骰后事件。旧文没有进一步记下这两项取舍的原因。首次战斗、回合结束抽牌等流程的位置由核心指令的组合决定，不由事件自行调度。
