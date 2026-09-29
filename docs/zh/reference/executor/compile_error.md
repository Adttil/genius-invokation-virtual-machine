[givm](../../reference.md) / [执行](../executor.md) / **compile_error**

# givm::compile_error

定义于头文件 `<givm/compile.hpp>`

```cpp
struct compile_error;
```

定义库编译时发现的一项错误，同时保存出错位置和具体原因。[`compile`](compile.md) 失败时返回此类型的列表。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `location` | [`compile_location`](compile_location.md) | 发生阶段及源、程序、命令位置 |
| `reason` | [`compile_error_reason`](compile_error_reason.md) | 具体错误的 variant |

## 注意

诊断拥有所需的名称字符串，不借用已经结束的编译上下文。按类型处理时使用 `std::get_if` 或 `std::visit`；只需文本时使用 [`error_string`](error_string.md)。
