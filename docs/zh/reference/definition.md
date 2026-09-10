[givm](../reference.md) / **定义**

# 定义

定义模块描述卡牌、角色及其他实体的规则，并准备一场对局所需的内容。这里的“定义”是同类实体共用的规则；牌桌上的某张卡牌、某个召唤物是采用定义的实体，各自的当前状态另由[牌桌模块](table.md)保存。

定义源适合按游戏内容逐项编写；源库负责汇集与选择，编译所得定义库用于对局。初次编写卡牌或角色效果可以先阅读[定义源协议](definition/source_protocol.md)。

## 定义的准备与使用

|  |  |
| --- | --- |
| [`definition_source_library`](definition/definition_source_library.md) | 汇集与编译定义源 |
| [`definition_source_view`](definition/definition_source_view.md) | 查看定义源的名称、分类和依赖 |
| [`definition_compile_context`](definition/definition_compile_context.md) | 编写定义时查找依赖并登记效果 |
| [`definition_selection`](definition/definition_selection.md) | 选择对局需要的内容 |
| [`definition_compile_result`](definition/definition_compile_result.md) | 取得定义库与配套 ID 映射 |
| [`definition_library`](definition/definition_library.md) | 查询对局使用的定义 |
| [`linked_deck`](definition/linked_deck.md) | 已确定卡牌与角色定义的牌组 |
| [`link_deck`](definition/link_deck.md) | 按名称准备牌组 |

## 名称、ID 与标签

|  |  |
| --- | --- |
| [`issued_id`](definition/issued_id.md) | 定义与标签 ID 的共同操作 |
| [`definition_id`](definition/definition_id.md) | 标识实体定义 |
| [`tag_id`](definition/tag_id.md) | 标识分类标签 |
| [`issued_id_map`](definition/issued_id_map.md) | 在名称和 ID 之间查询 |
| [`tag_mask`](definition/tag_mask.md) | 记录并匹配一组分类标签 |

## 效果与指令

|  |  |
| --- | --- |
| [`program_entry`](definition/program_entry.md) | 选择后续效果或终局结果 |
| [`execution_position`](definition/execution_position.md) | 表示当前将要执行的操作位置 |
| [`any_instruction_for`](definition/any_instruction_for.md) | 在同种 context 下保存不同指令 |
| [`instruction_compatible_with`](definition/instruction_compatible_with.md) | 检查指令与 context 的兼容性 |
| [`instruction_type_index`](definition/instruction_type_index.md) | 识别具体指令类型 |

## 定义类别与响应类型

|  |  |
| --- | --- |
| [`card_definition`](definition/card_definition.md) | 卡牌定义类别 |
| [`status_definition`](definition/status_definition.md) | 卡牌状态定义类别 |
| [`definition_types`](definition/definition_types.md) | 全部定义类别 |
| [`views_of_definition`](definition/views_of_definition.md) | 定义对应的实体形态 |
| [`subscribed_events`](definition/subscribed_events.md) | 实体形态可响应的事件 |
| [`support_subscribed_events`](definition/support_subscribed_events.md) | 场上实体共用的事件列表 |
| [`handler_program_context`](definition/handler_program_context.md) | 事件响应 context 的映射 |
| [`handler_program_context_t`](definition/handler_program_context_t.md) | 事件响应所用的 context 类型 |
| [`handler_program_entry_t`](definition/handler_program_entry_t.md) | 事件响应返回的效果入口类型 |
| [`definition_data`](definition/definition_data.md) | 持有已编译定义的数据 |
| [`handle_fn_t`](definition/handle_fn_t.md) | 统一的事件响应函数指针类型 |
