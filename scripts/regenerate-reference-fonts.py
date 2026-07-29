#!/usr/bin/env python3
"""Regenerate Pyramid's owned reference TTF and high-resolution .pfont assets."""
from __future__ import annotations

import argparse
import hashlib
import runpy
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
ASSET_DIR = ROOT / "Examples" / "BasicGame" / "Assets" / "Fonts"
LATIN_SCRIPT = Path(__file__).with_name("generate-pyramid-font.py")
ARABIC_SCRIPT = Path(__file__).with_name("generate-pyramid-arabic-font.py")
ASSET_NAMES = (
    "PyramidSans.ttf",
    "PyramidSans-64-sdf.pfont",
    "PyramidArabic.ttf",
    "PyramidArabic-64-sdf.pfont",
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def compile_assets(compiler: Path, output: Path) -> None:
    output.mkdir(parents=True, exist_ok=True)
    latin = runpy.run_path(str(LATIN_SCRIPT))
    arabic = runpy.run_path(str(ARABIC_SCRIPT))

    latin_ttf = output / "PyramidSans.ttf"
    arabic_ttf = output / "PyramidArabic.ttf"
    latin["build"](latin_ttf)
    arabic["build"](arabic_ttf)

    subprocess.run([
        str(compiler), str(latin_ttf), str(output / "PyramidSans-64-sdf.pfont"),
        "64", "1024", "--sdf", "--distance=10",
        "U+00E9", "U+03A9", "U+2713"], check=True)

    arabic_extras = [
        codepoint for codepoint in arabic["SUPPORTED_CODEPOINTS"]
        if not 32 <= codepoint <= 126
    ]
    subprocess.run([
        str(compiler), str(arabic_ttf), str(output / "PyramidArabic-64-sdf.pfont"),
        "64", "1024", "--sdf", "--distance=10",
        *(f"U+{codepoint:04X}" for codepoint in arabic_extras)], check=True)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--compiler", required=True, type=Path,
        help="Path to PyramidFontCompiler or PyramidFontCompiler.exe")
    parser.add_argument(
        "--check", action="store_true",
        help="Regenerate in a temporary directory and compare with checked-in assets")
    args = parser.parse_args()

    compiler = args.compiler.resolve()
    if not compiler.is_file():
        parser.error(f"compiler does not exist: {compiler}")

    if args.check:
        with tempfile.TemporaryDirectory(prefix="pyramid-fonts-") as temporary:
            generated = Path(temporary)
            compile_assets(compiler, generated)
            mismatches = []
            for name in ASSET_NAMES:
                expected = ASSET_DIR / name
                actual = generated / name
                if not expected.is_file() or expected.read_bytes() != actual.read_bytes():
                    mismatches.append(name)
            if mismatches:
                print("Reference font assets are stale: " + ", ".join(mismatches))
                return 1
            print("Reference font assets reproduce byte-for-byte.")
    else:
        compile_assets(compiler, ASSET_DIR)
        for name in ASSET_NAMES:
            path = ASSET_DIR / name
            print(f"{name}: {sha256(path)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
