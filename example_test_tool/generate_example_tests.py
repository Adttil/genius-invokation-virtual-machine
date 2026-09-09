"""Generate Catch2 tests from C++ examples authored in Markdown.

The example section contains a cpp code block followed by a text output block.

Each reference document contains at most one example, named by its path relative
to docs/zh/reference/ without the .md suffix. All examples become Catch2 tests in one
generated translation unit. Markdown is the sole source. Templates are excluded.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass, replace
from pathlib import Path
import re
import sys


class ExampleError(Exception):
    """A documentation example failed validation."""


@dataclass(frozen=True)
class Example:
    document: Path
    name: str
    code: str
    expected: str
    line: int
    code_line: int

    @property
    def location(self) -> str:
        return f"{self.document}:{self.line}"


EXAMPLE_SECTION = re.compile(r"^## 示例[ \t]*$", re.MULTILINE)
NEXT_SECTION = re.compile(r"^#{1,2}[ \t]+", re.MULTILINE)
CPP_BLOCK = re.compile(r"^```cpp[ \t]*\n(.*?)^```[ \t]*$", re.MULTILINE | re.DOTALL)
TEXT_BLOCK = re.compile(r"^```text[ \t]*\n(.*?)^```[ \t]*$", re.MULTILINE | re.DOTALL)
CPP_LITERAL_OR_COMMENT = re.compile(
    r'//[^\n]*|/\*.*?\*/|R"(?P<delimiter>[^\s()\\]{0,16})\(.*?\)(?P=delimiter)"'
    r'|"(?:\\.|[^"\\])*"|(?<![\w.])(?:u8|u|U|L)?\'(?:\\.|[^\'\\\n])*\'',
    re.DOTALL,
)
INCLUDE = re.compile(r'\s*#\s*include\s*(<[^>\n]+>|"[^"\n]+")\s*')


def normalize_newlines(text: str) -> str:
    return text.replace("\r\n", "\n")


def read_text(path: Path) -> str:
    return normalize_newlines(path.read_bytes().decode("utf-8"))


def parse_document(document: Path) -> list[Example]:
    text = read_text(document)
    section = EXAMPLE_SECTION.search(text)
    if section is None:
        return []
    next_section = NEXT_SECTION.search(text, section.end())
    end = next_section.start() if next_section else len(text)
    line = text.count("\n", 0, section.start()) + 1
    code = CPP_BLOCK.search(text, section.end(), end)
    if code is None:
        raise ExampleError(f"{document}:{line}: example section needs a cpp block")
    output = TEXT_BLOCK.search(text, code.end(), end)
    if output is None:
        raise ExampleError(f"{document}:{line}: example section needs a text block after its cpp block")
    return [Example(
        document, document.stem, code.group(1), output.group(1), line,
        text.count("\n", 0, code.start(1)) + 1,
    )]


def collect_examples(root: Path) -> list[Example]:
    reference = root / "docs" / "zh" / "reference"
    templates = reference / "templates"
    examples = []
    for document in sorted(reference.rglob("*.md")):
        if not document.is_relative_to(templates):
            name = document.relative_to(reference).with_suffix("").as_posix()
            examples.extend(replace(example, name=name) for example in parse_document(document))
    if not examples:
        raise ExampleError("No documented examples found")
    return examples


def mask_cpp(code: str, *, literals: bool) -> str:
    """Hide comments/literals while preserving line and character positions."""
    def replace(match: re.Match) -> str:
        token = match.group()
        if literals or token.startswith(("//", "/*")):
            return re.sub(r"[^\n]", " ", token)
        return token

    return CPP_LITERAL_OR_COMMENT.sub(replace, code)


def split_example(example: Example) -> tuple[list[str], str]:
    """Lift only unconditional leading includes; keep the C++ body unchanged."""
    includes = []
    body = []
    leading = True
    without_comments = mask_cpp(example.code, literals=False).splitlines(keepends=True)
    without_literals = mask_cpp(example.code, literals=True).splitlines(keepends=True)
    lines = example.code.splitlines(keepends=True)
    for offset, (line, visible, tokens) in enumerate(zip(lines, without_comments, without_literals)):
        if tokens.lstrip().startswith("#"):
            include = INCLUDE.fullmatch(visible.rstrip("\n"))
            if not leading or include is None:
                raise ExampleError(
                    f"{example.document}:{example.code_line + offset}: "
                    "only unconditional #include directives before the example body are supported"
                )
            includes.append(f"#include {include.group(1)}")
            # Leave comments and line positions intact, including block comments
            # that start or end on the same line as an include.
            body.append("".join(
                original if masked.isspace() else " "
                for original, masked in zip(line, visible)
            ))
        else:
            if visible.strip():
                leading = False
            body.append(line)
    return includes, "".join(body)


def complete_main(example: Example, body: str) -> str:
    """Preserve main's implicit success return after wrapping it in a namespace."""
    tokens = mask_cpp(body, literals=True).rstrip()
    message = f"{example.location}: the last definition must be int main()"
    if not tokens.endswith("}"):
        raise ExampleError(message)

    # Locate the opening brace paired with the final real closing brace.
    # Comments and literals have been masked, so their braces do not count.
    depth = 0
    for opening in range(len(tokens) - 1, -1, -1):
        if tokens[opening] == "}":
            depth += 1
        elif tokens[opening] == "{":
            depth -= 1
            if depth == 0:
                break
    else:
        raise ExampleError(message)
    if not re.search(r"\bint\s+main\s*\(\s*\)\s*$", tokens[:opening]):
        raise ExampleError(message)

    closing = len(tokens) - 1
    return body[:closing] + "\n    return 0;\n" + body[closing:]


def write_if_changed(path: Path, text: str) -> bool:
    content = text.encode("utf-8")
    if path.exists() and path.read_bytes() == content:
        return False
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)
    return True


def cpp_string(text: str) -> str:
    """Encode exact UTF-8 bytes, including NUL, without C++ escape ambiguity."""
    escapes = {10: r"\n", 13: r"\r", 9: r"\t", 34: r'\"', 92: r"\\"}
    return '"' + "".join(
        escapes.get(byte, chr(byte) if 32 <= byte < 127 else f"\\{byte:03o}")
        for byte in text.encode("utf-8")
    ) + '"'


def generate_examples(examples: list[Example], output_dir: Path) -> int:
    includes: dict[str, None] = {}
    bodies = []
    for index, example in enumerate(examples):
        headers, body = split_example(example)
        body = complete_main(example, body)
        includes.update(dict.fromkeys(headers))
        document = cpp_string(example.document.resolve().as_posix())
        bodies.append(
            f"namespace givm_doc_example_{index}\n{{\n"
            f'#line {example.code_line} {document}\n'
            f'{body}\n'
            f'#line {example.line} {document}\n'
            f'TEST_CASE({cpp_string(example.name)}, "[example]")\n{{\n'
            f'    ::givm::examples::check(&givm_doc_example_{index}::main,\n'
            f'        ::std::string_view{{{cpp_string(example.expected)}, '
            f'{len(example.expected.encode("utf-8"))}}});\n'
            f'}}\n}}\n#line 1 "givm_generated_examples"\n'
        )
    includes.update(dict.fromkeys(('#include "example_test_tools.hpp"',)))
    source = "// Generated from documentation; edit the Markdown examples.\n"
    source += "\n".join(includes) + "\n\n" + "\n".join(bodies)
    source += "\nint main(int argc, char* argv[])\n{\n"
    source += "    return givm::examples::run_main(argc, argv);\n}\n"
    return int(write_if_changed(output_dir / "examples.cpp", source))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output-dir", required=True, type=Path)
    args = parser.parse_args()
    try:
        examples = collect_examples(args.root.resolve())
        changed = generate_examples(examples, args.output_dir.resolve())
        print(f"Generated {len(examples)} example(s); updated {changed} build file(s).")
        return 0
    except (ExampleError, OSError, UnicodeDecodeError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
