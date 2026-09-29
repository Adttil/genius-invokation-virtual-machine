[givm](../../reference.md) / [执行](../executor.md) / **definition_compile_result**

# givm::definition_compile_result

定义于头文件 `<givm/executor.hpp>`

```cpp
struct definition_compile_result;
```

成功编译一套游戏规则的结果，包含可用于对局的定义库，以及把定义和标签名称转换为本次编译 ID 的映射。

## 成员对象

| 名称 | 类型 | 说明 |
| --- | --- | --- |
| `library` | [`definition_library`](definition_library.md) | 编译后的实体定义与对局流程 |
| `id_map` | [`issued_id_map`](../definition/issued_id_map.md) | 同一次编译产生的定义及标签名称映射 |

## 注意

两者对应同一个定义集合和 ID 分配结果，按上述顺序支持结构化绑定。通过 [`compile`](compile.md) 返回的 `expected` 确认编译成功后，可以使用 `result->library`、`result->id_map`，或以 `auto [library, id_map] = std::move(*result);` 取得两者。

## 参阅

|  |  |
| --- | --- |
| [`compile`](compile.md) | 编译定义源与对局流程 |
