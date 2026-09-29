[givm](../../reference.md) / [定义](../definition.md) / **source_conflict**

# givm::source_conflict

定义于头文件 `<givm/source_library.hpp>`

```cpp
struct source_conflict;
```

登记或合并定义源时，同类别同名项指向不同对象或具有不同类型的诊断。相同对象且类型相同的重复登记会被去重，不产生本诊断。

## 成员类型

| | |
| --- | --- |
| [`reason`](source_conflict/reason.md) | 同名源无法去重的原因 |

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `definition` | [`definition_name`](definition_name.md) | 发生冲突的定义类别和名称 |
| `cause` | `reason` | 对象不同或类型不同的冲突原因 |
| `first_input_index` | `std::optional<std::size_t>` | 首个同名项的参数索引；为空表示接收库中的源 |
| `second_input_index` | `std::optional<std::size_t>` | 后续冲突项的参数索引；为空表示被合并库中的源 |

## 非成员函数

| | |
| --- | --- |
| [`error_string`](error_string.md) | 将诊断列表转换为可读文本 |

## 注意

参数索引从零开始。批量输入之间冲突时两个索引均有值；输入与接收库冲突时只有第二个索引有值；合并两个源库时两个索引均为空。

先比较源的 C++ 类型，类型不同时 `cause` 为 `different_type`；类型相同但对象不同时为 `different_object`。对于动态源，同样比较传入的 C++ 源对象及类型，不推断其引用的脚本定义是否相同。

## 参阅

| | |
| --- | --- |
| [`definition_source_library::add`](definition_source_library/add.md) | 登记定义源或合并源库，并返回诊断 |
