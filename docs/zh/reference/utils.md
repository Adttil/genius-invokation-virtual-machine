[givm](../reference.md) / **通用工具**

# 通用工具

用于保存临时数据和组织编译期类型的工具。

## 类

|  |  |
| --- | --- |
| [`frame_stack`](utils/frame_stack.md) | 保存临时数据的栈 |
| [`frame_view`](utils/frame_view.md) | 普通帧的访问对象 |
| [`frame_t`](utils/frame_t.md) | 帧的类型描述 |
| [`substack_t`](utils/substack_t.md) | 帧尾子栈的类型标记 |
| [子栈视图](utils/substack_view.md) | 访问帧中另一个栈的内容 |
| [`type_list`](utils/type_list.md) | 编译期的有序类型序列 |

## 类型别名

|  |  |
| --- | --- |
| [`stack_count_t`](utils/stack_count_t.md) | 动态数组的数量类型 |
| [`type_list_cat`](utils/type_list_cat.md) | 多组类型序列按顺序拼接所得的类型 |
| [`maybe_mutable`](utils/maybe_mutable.md) | 由访问权限决定 const 限定的类型 |

## 常量

|  |  |
| --- | --- |
| [`frame`](utils/frame.md) | 用于多帧操作的描述常量 |
| [`dynamic_array`](utils/dynamic_array.md) | 动态数组初始化参数的构造工具，是一个可调用常量 |
| [`max_alignment`](utils/max_alignment.md) | 栈支持的对齐上限 |

## 函数

|  |  |
| --- | --- |
| [`get`](utils/get.md) | 访问帧中的元素 |
| [`substack`](utils/substack.md) | 构造空子栈的初始化参数 |
