[开发备忘](../notes.md) / **示例测试工具**

# 示例测试工具

示例源码和预期输出以 Markdown 为唯一来源。编写要求见[Reference 文档规范](../reference_stype.md#示例)，这里说明聚合编译、输出检查和构建方式。

## 聚合成一个编译单元

[`generate_example_tests.py`](../../../example_test_tool/generate_example_tests.py) 扫描 `docs/zh/reference/` 下的 Markdown。每页最多一个 `## 示例`；从该节提取第一个 `cpp` 块及其后的第一个 `text` 块，缺少任一块时报告格式错误。没有示例节的页面及其他章节的声明不参与提取，规范和模板位于扫描范围之外。

生成器把所有前导 `#include` 提到聚合文件开头，其他代码分别放入独立命名空间。这样重模板头文件只需在一个编译单元中处理，各示例也可以使用相同的辅助名称。示例最后的 `main` 会转换为普通可调用函数，并在末尾补上 `return 0;`；只修改生成内容，不回写文档。

每个页面生成一个带 `[example]` 标签的 Catch2 用例，名称是文档相对于 `docs/zh/reference/` 的路径去掉 `.md`，例如 `executor/executor/step` 或 `executor/instructions/draw_cards`。移动页面会改变测试名，无需另行维护登记表。

需要扩展全局 `std` 或 `givm` 命名空间的示例不能直接放入这一聚合方式。产生输出等副作用的初始化应在 `main` 或其调用的函数中完成，不能放在命名空间作用域初始化时执行。

## 构建与运行

示例测试需要支持 C++23 的编译器、Python 3.10 或更新版本，以及项目配置的 Catch2。生成器仅使用 Python 标准库。

| Preset | 用途 | 构建的测试 |
| --- | --- | --- |
| `givm` | 仅使用本库 | 无 |
| `givm-tests` | 开发内部实现 | 单元测试 |
| `givm-examples` | 编写文档 | 文档示例 |
| `givm-tests-examples` | 修改公开行为并同步文档 | 单元测试和文档示例 |

在仓库根目录执行：

```text
cmake --preset givm-examples -B out/build/examples
cmake --build out/build/examples --config Debug --target givm_examples
ctest --test-dir out/build/examples -C Debug -L "^example$" --output-on-failure
```

配置阶段只建立构建规则。构建 `givm_examples` 时生成聚合源码并按需编译，不自动运行测试。CTest 为每个示例启动独立进程，设置 10 秒超时；用例内检查返回值和标准输出，失败时报告原因。输出比较只统一 CRLF/LF，其他空白及末尾换行均参与比较。

也可直接运行 `givm_examples`：无参数检查全部示例，以文档相对路径选择单个示例，末尾可保留 `.md`。下面的路径适用于可执行文件位于 `out/build/examples/example_test_tool/` 的构建：

```text
out/build/examples/example_test_tool/givm_examples
out/build/examples/example_test_tool/givm_examples executor/executor/step.md
out/build/examples/example_test_tool/givm_examples --list-tests
```

Windows 文件名带 `.exe`；使用多配置生成器时，路径中还可能包含 `Debug/` 等配置目录。也可以使用 Catch2 的 `[example]` 标签筛选。

直接运行全部用例时共用一个进程；需要崩溃隔离和超时保护时使用 CTest。标准输出由[测试支持代码](../../../example_test_tool/example_test_tools.hpp)捕获，测试报告写入标准错误，文档内无需逐例改写输出代码。

## 增量构建

构建规则及支持代码集中在 `example_test_tool/`。每个构建目录分别生成自己的 `example_test_tool/generated/examples.cpp`；`out/` 已被忽略，不提交生成文件。生成成本很低，各 preset 不共用可写产物，避免并行构建时互相覆盖。

编辑、增加或删除文档，以及修改生成器，都不需要 CMake 重新配置。每次构建 `givm_examples` 重新扫描；内容不变时不重写 CPP，避免无意义重编译。修改预期输出必须由文档作者确认，验证过程不会自动把实际输出写回文档。
