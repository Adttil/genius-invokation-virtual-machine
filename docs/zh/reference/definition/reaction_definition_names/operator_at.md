[givm](../../../reference.md) / [定义](../../definition.md) / [reaction_definition_names](../reaction_definition_names.md) / **operator[]**

# givm::reaction_definition_names::operator[]

```cpp
constexpr std::string_view& operator[](elemental_reaction slot) noexcept;
constexpr const std::string_view& operator[](elemental_reaction slot) const noexcept;
```

以基础反应槽位读取或设置名称，例如 `names[elemental_reaction::electro_charged] = "custom-reaction"`。不接受数值下标。`slot` 必须为有效且非 none 的反应枚举，不进行运行期检查。
