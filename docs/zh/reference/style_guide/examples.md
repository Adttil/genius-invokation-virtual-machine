[GIVM](../../reference.md) / [写作规范](../style_guide.md)

# 示例维护与验证

示例源码和预期输出直接写在文档中。构建时将全部示例汇总到构建目录中的 `example_test_tool/generated/examples.cpp`，每个含示例的页面生成一个带 `[example]` 标签的 Catch2 测试用例，编译为 `givm_examples`。仓库中的 `example_test_tool/` 保存测试支持代码和构建规则，生成文件不提交到仓库。

汇总时，前导 `#include` 放在文件开头，各示例的其余内容放进独立命名空间。重模板头文件只需在一个编译单元中处理，示例之间也可以使用相同的辅助函数名和类型名。

## 文档结构

每页至多设置一个 `## 示例` 栏目，并在其中放一个完整示例。生成器读取该节的第一个 `cpp` 代码块作为源码，再读取其后的第一个 `text` 代码块作为预期输出；两者之间写一行“输出”。缺少 `cpp` 块或后续 `text` 块时，生成器会报错。需要展示多个用法时，在同一个 `main` 中分段演示。完整结构见[函数页模板](../templates/function.md)，实际用法见 [`position`](../executor/executor/position.md)。

测试名称由文档相对于 `docs/zh/reference/` 的路径去掉 `.md` 得到。例如 `docs/zh/reference/executor/executor/position.md` 对应 `executor/executor/position`，移动文档会自动改变名称。

只扫描 `docs/zh/reference/`，其中 `templates/` 的占位示例不参与验证。没有 `## 示例` 节的页面不生成测试用例，其他章节中的声明代码块不参与示例提取。

## 修改示例

直接修改文档中的 `cpp` 代码块。输出发生有意变化时，检查新行为并修改 `text` 块。验证过程不会自动改写预期输出。

完整示例遵循以下约定：

- 使用 `int main()`，并将它作为示例最后一个定义；之后可以保留注释。文档中可省略结尾的 `return 0;`，生成器会在汇总后的函数末尾补入，只修改构建产物。
- 将所有无条件 `#include` 放在源码开头。正文不使用宏定义、条件编译等预处理指令，生成器会检查这一约定。
- 辅助函数和类型应适合放进一个命名空间。需要扩展 `std` 或 `givm` 等全局命名空间的代码不适合这一汇总方式。产生输出等副作用的初始化放在 `main` 或它调用的函数中。
- 通过 `std::cout` 或普通 `std::print`、`std::println` 输出，保持标准流的默认同步设置，不自行重定向或关闭标准流。示例的标准输出由测试捕获，测试报告写入标准错误。

## 编译与运行

根据本次工作选择 CMake preset：

| Preset | 用途 | 构建的测试 |
| --- | --- | --- |
| `givm` | 仅使用本库 | 无 |
| `givm-tests` | 开发内部实现 | 单元测试 |
| `givm-examples` | 编写文档 | 文档示例 |
| `givm-tests-examples` | 修改公开行为并同步文档 | 单元测试和文档示例 |

示例测试需要支持 C++23 的编译器、Python 3.10 或更新版本，以及项目配置的 Catch2。`example_test_tool/generate_example_tests.py` 仅使用 Python 标准库，负责提取示例并生成聚合源码。在仓库根目录执行：

```text
cmake --preset givm-examples -B out/build/examples
cmake --build out/build/examples --config Debug --target givm_examples
ctest --test-dir out/build/examples -C Debug -L "^example$" --output-on-failure
```

CMake 配置时只建立构建规则。构建 `givm_examples` 时先调用生成器，再按需编译 `examples.cpp`，不自动运行测试。CTest 为每个示例启动独立进程并设置超时；示例的返回值和标准输出由 C++ 测试检查，失败时报告原因。

也可以直接运行 `givm_examples`。无参数时校验全部示例，以文档相对路径为位置参数可选择一个用例，末尾的 `.md` 可保留。以下路径适用于可执行文件位于 `out/build/examples/example_test_tool/` 的构建：

```text
out/build/examples/example_test_tool/givm_examples
out/build/examples/example_test_tool/givm_examples executor/executor/position.md
out/build/examples/example_test_tool/givm_examples --list-tests
```

Windows 上可执行文件带有 `.exe` 后缀；使用 Visual Studio 等多配置生成器时，还需在路径中加入 `Debug/` 等配置目录。每个示例带有 `[example]` 标签，也可使用 Catch2 的标签筛选。

直接运行全部示例时，各用例共用一个进程。需要逐项隔离及超时保护时，使用 CTest。

修改文档代码、预期输出，或增删示例页面后，下一次构建会自动更新测试。比较输出时只统一 CRLF/LF 换行符，其他空白和末尾换行均参与比较。

## 文件位置与增量构建

示例构建所需的文件集中在 `example_test_tool/`：`generate_example_tests.py` 生成源码，`example_test_tools.hpp` 提供输出捕获和 Catch2 入口，`CMakeLists.txt` 组织构建与测试。

聚合源码保存在各自构建目录的 `example_test_tool/generated/examples.cpp` 中，例如 `out/build/examples/example_test_tool/generated/examples.cpp`。`out/` 已由 `.gitignore` 忽略，生成文件不提交。

编辑、增加或删除文档，以及修改生成器，都不触发 CMake 重新配置。每次构建 `givm_examples` 时，生成器重新扫描文档，因此新增和删除的示例也会自动生效。生成内容不变时不重写 CPP，避免无意义的重编译；示例格式错误会在构建时报告。

各构建目录分别生成自己的副本，生成成本很低，也避免了不同 preset 并行构建时共用可写文件。
