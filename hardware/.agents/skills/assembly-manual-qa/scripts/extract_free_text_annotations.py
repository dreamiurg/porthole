"""Print PDF FreeText annotations as JSON, preserving page numbers."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from pypdf import PdfReader


def annotation_text(value: object) -> str:
    if isinstance(value, str):
        return value
    if isinstance(value, bytes):
        if value.startswith(b"\xfe\xff"):
            return value[2:].decode("utf-16-be", errors="replace")
        if value.startswith(b"\xff\xfe"):
            return value[2:].decode("utf-16-le", errors="replace")
        return value.decode("utf-8", errors="replace")
    return str(value)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("pdf", type=Path)
    args = parser.parse_args()
    reader = PdfReader(str(args.pdf))
    rows = []
    for page_number, page in enumerate(reader.pages, start=1):
        for annotation in page.get("/Annots", []):
            item = annotation.get_object()
            if item.get("/Subtype") != "/FreeText":
                continue
            rows.append(
                {
                    "page": page_number,
                    "contents": annotation_text(item.get("/Contents", "")),
                    "rect": list(item.get("/Rect", [])),
                }
            )
    print(json.dumps(rows, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
