[givm](../../../reference.md) / [执行](../../executor.md) / [definition_library](../definition_library.md) / **shield_id**

# givm::definition_library::shield_id

定义于头文件 `<givm/runtime.hpp>`

```cpp
definition_id<combat_status_view> shield_id() const noexcept;
```

取得默认结晶反应所采用的护盾定义。该定义由编译时的 [`basic_definition_sources`](../../definition/basic_definition_sources.md) 的 `shield` 指定，可以是随库提供的版本，也可以是自定义版本。

## 返回值

本定义库中的有效 `definition_id<combat_status_view>`。即使编译时只选择了部分定义，该定义也会保留。

## 注意

返回定义 ID，不创建护盾实体。默认结晶反应在反应目标的对方请求生成一点护盾；重复生成、抵消伤害和耗尽离场由定义源实现。随库提供的 [`shield_3_3_0`](../../basic_definitions.md#结晶与护盾) 通常最多累加两层，抵消己方出战角色受到的非穿透伤害。
