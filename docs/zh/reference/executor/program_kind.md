[givm](../../reference.md) / [执行](../executor.md) / **program_kind**

# givm::program_kind

定义于头文件 `<givm/compile.hpp>`

```cpp
enum class program_kind { initialization, round, response };
```

编译诊断中的程序类别。

## 枚举项

| | |
| --- | --- |
| `initialization` | 对局开始时执行一次的初始化程序 |
| `round` | 每个回合执行的根程序 |
| `response` | 定义源通过 `add_normal_effect` 登记的响应程序 |
