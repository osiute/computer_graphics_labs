#!/usr/bin/env python3
"""Run real CLI checks and save demonstrable PNGs/logs; Python standard library only."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main():
    root = Path(__file__).resolve().parent
    binary = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root / "build/lab16_app"
    output = Path(sys.argv[2]).resolve() if len(sys.argv) > 2 else root / "results"
    output.mkdir(parents=True, exist_ok=True)
    log = []
    checks = 0

    def run(name, args, expected=0, cwd=root, stdin=None, contains=None):
        nonlocal checks
        command = [str(binary), *map(str, args)]
        result = subprocess.run(command, input=stdin, text=True, encoding="utf-8",
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                cwd=cwd, timeout=30)
        log.extend([f"=== {name} ===", f"argv: {command!r}", f"cwd: {cwd}"])
        if stdin is not None:
            log.append(f"stdin: {stdin!r}")
        log.extend([result.stdout.rstrip(), f"exit: {result.returncode}", ""])
        if result.returncode != expected:
            raise AssertionError(f"{name}: exit {result.returncode}, expected {expected}")
        if contains is not None and contains not in result.stdout:
            raise AssertionError(f"{name}: expected output {contains!r}")
        checks += 1
        print(f"PASS: {name}")

    try:
        assets = root / "assets"
        run("standard house", [assets / "house.obj", output / "house.png", 35, 25, 1000, 800])
        run("different camera", [assets / "house.obj", output / "house_other.png", -35, 15])
        assert (output / "house.png").read_bytes() != (output / "house_other.png").read_bytes()
        with tempfile.TemporaryDirectory(prefix="lab16-cli-") as temporary:
            temporary = Path(temporary)
            modified = temporary / "changed material"
            modified.mkdir()
            shutil.copyfile(assets / "house.obj", modified / "house.obj")
            material = (assets / "house.mtl").read_text(encoding="utf-8")
            assert "Kd 0.72 0.24 0.16" in material
            (modified / "house.mtl").write_text(material.replace("Kd 0.72 0.24 0.16", "Kd 0.15 0.75 0.25"),
                                                 encoding="utf-8")
            run("changed Roof Kd, new process", [modified / "house.obj", output / "house_roof_green.png",
                                                35, 25, 1000, 800], contains="Roof: RGB(38, 191, 64)")
            assert (output / "house.png").read_bytes() != (output / "house_roof_green.png").read_bytes()
            # No modification of the original OBJ/MTL is needed for these cases.
            shutil.copyfile(assets / "house.obj", temporary / "house.obj")
            run("missing MTL fallback", [temporary / "house.obj", output / "house_without_mtl.png"],
                contains="Предупреждение:")
            run("missing OBJ", [temporary / "absent.obj", output / "absent.png"], expected=1,
                contains="Не удалось открыть OBJ")
            invalid = temporary / "bad.obj"
            invalid.write_text("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 0 2 3\n", encoding="utf-8")
            run("zero OBJ index", [invalid, output / "bad.png"], expected=1, contains="некорректный индекс")
            invalid.write_text("v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 99\n", encoding="utf-8")
            run("out-of-range OBJ index", [invalid, output / "bad.png"], expected=1, contains="вне диапазона")
            run("missing output directory", [assets / "house.obj", temporary / "absent" / "out.png"],
                expected=1, contains="Не удалось создать PNG")
            run("invalid elevation", [assets / "house.obj", temporary / "bad.png", 35, 91], expected=1)
            run("invalid dimensions", [assets / "house.obj", temporary / "bad.png", 35, 25, 0, 800], expected=1)
            run("invalid numeric input", [assets / "house.obj", temporary / "bad.png", "nan", 25], expected=1)
            run("interactive default model from another directory", [], cwd=temporary,
                stdin="\ninteractive.png\n35\n25\n320\n240\n", contains="PNG сохранён:")
            assert (temporary / "interactive.png").read_bytes().startswith(b"\x89PNG\r\n\x1a\n")
            run("interactive invalid path", [], cwd=temporary, stdin="absent.obj\n", expected=1,
                contains="Не удалось открыть OBJ")
            run("truncated interactive input", [], cwd=temporary, stdin="\n", expected=1,
                contains="Ввод завершён")
        run("help", ["--help"])
        log.append(f"PASS: {checks} real-process CLI checks. Blender was not run.")
        print(log[-1])
    finally:
        (output / "cli_checks.txt").write_text("\n".join(log) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
