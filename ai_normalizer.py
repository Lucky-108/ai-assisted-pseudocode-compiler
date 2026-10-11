import subprocess
from pathlib import Path
import json
import re
import urllib.request
import urllib.error
import sys

OLLAMA_URL = "http://172.22.64.1:11434/api/generate"

MODEL = "qwen2.5:1.5b"

GRAMMAR_RULES = """
Convert the user's instruction into canonical pseudocode.

Supported syntax:
INT x;
FLOAT y;
x = 1;
PRINT x;
IF (x > 5) { PRINT x; } ELSE { PRINT 0; }
WHILE (x <= 3) { PRINT x; x = x + 1; }

Rules:
- Declare every variable before use.
- Use uppercase keywords.
- End declarations, assignments, and PRINT statements with semicolons.
- Put conditions in parentheses and blocks in braces.
- Never use markdown fences, explanations, or END WHILE.
- Return pseudocode only.
- Every program must be complete and executable.
- Declare every variable before its first assignment or use.
- For example, if the instruction starts x at 1, output:
  INT x;
  x = 1;
- Never omit declarations or initial assignments.
- Before returning, check that every used variable is declared.

Program completeness requirements:
- Translate the ENTIRE instruction, not just its first sentence.
- Never omit a requested operation, condition, branch, loop, or print statement.
- If the user requests IF/ELSE, you MUST generate both the IF block and ELSE block.
- Preserve the order of the requested operations.
- Declare every variable before using it.
- Include initialization assignments when the instruction specifies initial values.
- Do not return a partial program just because it is syntactically valid.

Example:
Instruction: Create an integer variable x and set it to 10.
If x is greater than 5, print 1; otherwise, print 0.

Output:
INT x;
x = 10;
IF (x > 5) {
    PRINT 1;
} ELSE {
    PRINT 0;
}

Before responding, check that your output represents every part
of the instruction. Return only the complete pseudocode.

"""


def normalize(instruction: str) -> str:
    payload = {
        "model": MODEL,
        "prompt": (
            GRAMMAR_RULES
            + "\nInstruction:\n"
            + instruction
            + "\n\nCanonical pseudocode:"
        ),
        "stream": False,
        "options": {
            "temperature": 0,
            "num_predict": 256
        }
    }

    request = urllib.request.Request(
        OLLAMA_URL,
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST"
    )

    try:
        with urllib.request.urlopen(request, timeout=120) as response:
            result = json.loads(response.read().decode("utf-8"))
    except (urllib.error.URLError, TimeoutError) as error:
        raise RuntimeError(
            f"Could not contact Ollama at {OLLAMA_URL}: {error}"
        ) from error

    pseudocode = result.get("response", "").strip()

    # Remove optional Markdown fences.
    pseudocode = re.sub(
        r"^\s*```[^\n]*\n?", "", pseudocode
    )
    pseudocode = re.sub(r"\n?```\s*$", "", pseudocode)

    if not pseudocode:
        raise ValueError("Ollama returned empty pseudocode.")

    return pseudocode



if __name__ == "__main__":
    project_root = Path(__file__).resolve().parent
    compiler_path = project_root / "build" / "pseudocode_compiler"

    print("\nDescribe your program in plain English.")
    print("Enter your complete instruction, then press Enter on an empty line:")

    lines = []

    while True:
        try:
            line = input()
        except EOFError:
            break

        if not line.strip():
            break

        lines.append(line)

    instruction = "\n".join(lines)

    if not instruction.strip():
        print("No instruction provided.")
        sys.exit(1)

    try:
        # 1. Convert natural language into pseudocode.
        pseudocode = normalize(instruction)

        print("\n" + "=" * 50)
        print("AI-GENERATED PSEUDOCODE")
        print("=" * 50)
        print(pseudocode)

        # 2. Check that the compiler executable exists.
        if not compiler_path.is_file():
            raise FileNotFoundError(
                f"Compiler not found at {compiler_path}. "
                "Build the compiler first."
            )

        # 3. Feed the generated pseudocode into our compiler.
        print("\n" + "=" * 50)
        print("RUNNING COMPILER PIPELINE")
        print("=" * 50)

        result = subprocess.run(
            [str(compiler_path)],
            input=pseudocode,
            text=True,
            capture_output=True,
            cwd=str(project_root),
            timeout=30,
        )

        # 4. Display the compiler's output and any errors.
        if result.stdout:
            print(result.stdout, end="")

        if result.stderr:
            print(result.stderr, end="", file=__import__("sys").stderr)

        if result.returncode != 0:
            print(
                f"\nCompilation pipeline failed "
                f"(exit code {result.returncode})."
            )
        else:
            print("\nEnd-to-end pipeline completed successfully!")

    except subprocess.TimeoutExpired:
        print("Compilation pipeline timed out.")
    except (RuntimeError, ValueError, FileNotFoundError) as error:
        print(f"\nError: {error}")

