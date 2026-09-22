[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **frozen_id**

# givm::definition_library::frozen_id

定义于头文件 `<givm/executor.hpp>`

```cpp
definition_id<attachment_view> frozen_id() const noexcept;
```

取得默认冻结反应所采用的角色附属定义。该定义由[源库构造时](../../definition/definition_source_library/constructor.md)的第四个参数指定，可以是随库提供的版本，也可以是自定义版本。

## 返回值

本定义库中的有效 `definition_id<attachment_view>`。即使编译时只选择了部分定义，该定义也会保留。

## 注意

返回定义 ID，不创建冻结附属。默认冻结反应通过该定义向发生反应的角色施加附属；控制分类与具体响应由定义源提供。随库提供的 [`frozen_3_3_0`](../../basic_definitions.md) 具有 `control` 标签。

## 参阅

| | |
| --- | --- |
| [`is_controlled`](is_controlled.md) | 检查角色当前是否受控，并展示回合开始时冻结解除的示例 |
