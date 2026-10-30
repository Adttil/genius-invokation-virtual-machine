# givm::perspective_table_view::player

定义于头文件 `<givm/table.hpp>`

```cpp
template<unsigned Player>
constexpr auto player() const;
```

`Player` 必须为0或1。返回 `perspective_player_view<Player, Role>`，归属及可读接口在编译期确定。
