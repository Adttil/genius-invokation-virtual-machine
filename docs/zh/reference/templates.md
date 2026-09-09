[GIVM](../reference.md)

# Reference 文档模板

以下六类 API 页使用对应模板。模板文件本身就是页面骨架，可直接预览或整页复制。`type`、`function`、`arg1` 等都是示意名称，编写文档时替换为实际接口及说明，并删除不适用的栏目。

导航和链接已写出 Markdown 格式。模板以 `docs/zh/reference/topic.md` 为主题页、`docs/zh/reference/topic/type.md` 为类页、`docs/zh/reference/topic/type/method1.md` 为成员页展示相对路径。编写文档时按实际名称和层级调整。

| | |
| --- | --- |
| [类](templates/class.md) | 类与类模板 |
| [枚举](templates/enum.md) | 枚举类型与枚举项 |
| [函数](templates/function.md) | 命名空间作用域的函数与函数模板 |
| [类型别名](templates/type_alias.md) | 类型别名与别名模板 |
| [成员函数](templates/member_function.md) | 成员函数、构造函数、析构函数和运算符 |
| [常量](templates/constant.md) | 公开常量与常量模板 |

其他页面自由安排内容和结构，每页至多设置一个“示例”栏目。若设置该栏目，仍须包含完整可运行的 `cpp` 代码块和对应的 `text` 输出块。

模板中的声明和示例代码只是示意结构；`templates/` 不参与示例编译。实际页面的示例验证方式见[示例维护与验证](style_guide/examples.md)。

统一措辞、栏目和链接规则见[写作规范](style_guide.md)。
