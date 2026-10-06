[givm](../reference.md) / **定义**

# 定义

定义模块描述卡牌、角色及其他实体的规则，并准备一场对局所需的内容。这里的“定义”是同类实体共用的规则；牌桌上的某张卡牌、某个召唤物是采用定义的实体，各自的当前状态另由[牌桌模块](table.md)保存。

定义源适合按游戏内容逐项编写；源库负责汇集和访问所需定义。[执行模块的编译函数](executor/compile.md)选择定义、建立名称与 ID 的映射，并将定义源和对局流程编译为对局使用的定义库。初次编写卡牌或角色效果可以先阅读[定义源协议](definition/source_protocol.md)。

`<givm/definition_source_interface.hpp>` 提供完整的源库接口，包括登记、合并、查询、基础定义配置和源准备诊断。提供源库工厂函数的内容库可以在其公共头中包含这个入口，使调用方直接使用返回的源库。编写定义源时使用 `<givm/definition_source.hpp>`，获得命令、事件、编译上下文与响应上下文；整库编译使用 `<givm/compile.hpp>`。原有 `<givm/definition.hpp>` 继续提供完整定义模块接口。

## 类

### 定义的准备与使用

|  |  |
| --- | --- |
| [`definition_source_library`](definition/definition_source_library.md) | 可供编译的定义源集合 |
| [`reaction_definition_names`](definition/reaction_definition_names.md) | 编译时按反应槽位指定默认反应定义名称 |
| [`definition_name`](definition/definition_name.md) | 定义类别及名称 |
| [`source_conflict`](definition/source_conflict.md) | 同类别同名源的冲突诊断 |
| [`source_missing_dependency`](definition/source_missing_dependency.md) | 定义源的缺失依赖诊断 |
| [`source_selection_error`](definition/source_selection_error.md) | 选定定义不存在的诊断 |
| [`deck_link_error`](definition/deck_link_error.md) | 牌组名称无法链接的诊断 |
| [`definition_source_view`](definition/definition_source_view.md) | 定义源的只读视图 |
| [`program_entry`](definition/program_entry.md) | 响应效果的入口 |
| [`program_inputs`](definition/program_inputs.md) | 已准备并拥有的程序输入 |
| [`fixed_defer_program_input`](definition/fixed_defer_program_input.md) | 固定延迟命令的编译用参数 |
| [`history_summary_definition`](definition/history_summary.md) | 对局历史摘要的定义类别 |
| [`history_scalar_field<T>`](definition/history_summary.md#类) | 历史摘要的标量字段描述 |
| [`history_array_field<T>`](definition/history_summary.md#类) | 历史摘要的数组字段描述 |

### 名称、ID 与标签

|  |  |
| --- | --- |
| [`issued_id_map`](definition/issued_id_map.md) | 定义名称、分类标签与 ID 的对应表 |
| [`tag_mask`](definition/tag_mask.md) | 分类标签集合 |

### 定义类别与响应映射

|  |  |
| --- | --- |
| [`views_of_definition`](definition/views_of_definition.md) | 定义对应的实体形态 |
| [`subscribed_events`](definition/subscribed_events.md) | 实体形态可响应的事件 |
| [`supported_queries`](definition/supported_queries.md) | 定义类别支持的查询 |

## 类型别名

|  |  |
| --- | --- |
| [`source_add_error`](definition/source_add_error.md) | 定义源登记诊断的 variant |
| [`source_preparation_error`](definition/source_preparation_error.md) | 定义选择及 ID 准备诊断的 variant |
| [`history_field_descriptor`](definition/history_summary.md#类型别名) | 历史摘要字段描述的 variant |
| [`any_command`](definition/any_command.md) | 核心命令 variant |
| [`definition_types`](definition/definition_types.md) | 全部定义类别 |
| [`definition_data`](definition/definition_data.md) | 已编译定义的数据对象 |
| [`handle_fn_t`](definition/handle_fn_t.md) | 统一的事件响应函数指针类型 |

## 函数

|  |  |
| --- | --- |
| [`make_definition_source_library`](definition/make_definition_source_library.md) | 创建源库并批量登记定义源 |
| [`error_string`](definition/error_string.md) | 将源准备、牌组链接或单命令诊断转换为文本 |
| [`link_deck`](definition/link_deck.md) | 按名称准备牌组 |
| [`query_default`](definition/query_default.md) | 定义源未提供查询时的默认结果 |
| [`defer_invoke`](definition/defer_invoke.md) | 准备延迟程序的入口及参数 |
| [`fixed_defer_invoke`](definition/fixed_defer_invoke.md) | 准备固定延迟命令的入口及编译校验信息 |
| [`pack_inputs`](definition/pack_inputs.md) | 将专用输入对象打包为拥有型输入 |
| [`concat_inputs`](definition/concat_inputs.md) | 按顺序合并已准备的输入片段 |

## [命令](definition/commands.md)

组合游戏流程和实体响应效果的核心操作。

## [命令输入](definition/command_inputs.md)

响应提交的动态参数，以及运行时确定长度的输入数组。

## [事件](definition/events.md)

对局中的响应时机与事件数据。

## [查询](definition/queries.md)

角色初始状态、卡牌初始费用与当前行动参数的合法性等规则信息。

## [历史摘要](definition/history_summary.md)

按定义依赖选择的累计历史，在通知开始时更新，供效果和上层读取。
