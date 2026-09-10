[givm](../reference.md) / **通用工具**

# 通用工具

用于保存临时数据和组织编译期类型的工具。

## 栈与帧

|  |  |
| --- | --- |
| [`frame_stack`](utils/frame_stack.md) | 按后进先出保存临时数据 |
| [`frame_view`](utils/frame_view.md) | 访问一组栈上数据 |
| [`frame_t`](utils/frame_t.md) | 帧的类型描述 |
| [`frame`](utils/frame.md) | 用于多帧操作的描述常量 |
| [`dynamic_array`](utils/dynamic_array.md) | 准备动态长度数组 |
| [`stack_count_t`](utils/stack_count_t.md) | 动态数组的数量类型 |
| [`max_alignment`](utils/max_alignment.md) | 栈支持的对齐上限 |
| [`get`](utils/get.md) | 访问帧中的元素 |

## 编译期类型

|  |  |
| --- | --- |
| [`type_list`](utils/type_list.md) | 按顺序组织类型 |
| [`type_list_cat`](utils/type_list_cat.md) | 连接类型序列 |
| [`maybe_mutable`](utils/maybe_mutable.md) | 根据权限选择只读或可修改类型 |
