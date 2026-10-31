#!/usr/bin/env python3
"""Report PlatformIO's firmware budget and estimated named flash contributors."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import subprocess

APP_SECTIONS = (".iram0.vectors", ".iram0.text", ".dram0.data", ".flash.text", ".flash.rodata")
RAM_SECTIONS = (".dram0.data", ".dram0.bss")
FLASH_SECTIONS = (".flash.text", ".flash.rodata")


def parse_sections(output):
    sections = {}
    for line in output.splitlines():
        if not line.lstrip().startswith(".") or line.rstrip().endswith(":"):
            continue
        fields = line.split()
        if len(fields) != 3 or not all(re.fullmatch(r"\d+", v) for v in fields[1:]):
            raise ValueError(f"Malformed size section: {line.strip()}")
        name, size, address = fields
        if name in sections:
            raise ValueError(f"Duplicate size section: {name}")
        sections[name] = {"size": int(size), "address": int(address)}
    missing = sorted(set(APP_SECTIONS + RAM_SECTIONS) - sections.keys())
    if missing:
        raise ValueError(f"Missing required size sections: {', '.join(missing)}")
    return sections


def namespace(name):
    scope = name.split("(", 1)[0]
    return scope.split("::", 1)[0] if "::" in scope else "<global>"


def parse_symbols(output, sections):
    aliases = {}
    ranges = [(sections[n]["address"], sections[n]["size"]) for n in FLASH_SECTIONS]
    for line in output.splitlines():
        if not line.strip():
            continue
        match = re.fullmatch(r"\s*([0-9a-fA-F]+)\s+([0-9a-fA-F]+)\s+(\w)\s+(.+)", line)
        if not match:
            if re.fullmatch(r"\s*[0-9a-fA-F]+\s+[a-zA-Z?]\s+.+", line):
                continue  # nm prints absolute symbols and unsized labels without a size column.
            raise ValueError(f"Malformed nm symbol: {line.strip()}")
        address, size, kind, name = match.groups()
        address, size = int(address, 16), int(size, 16)
        if not size or not any(start <= address and address + size <= start + length for start, length in ranges):
            continue
        key = address, size
        symbol = {"address": address, "size": size, "type": kind, "name": name, "namespace": namespace(name)}
        if key not in aliases or name < aliases[key]["name"]:
            aliases[key] = symbol
    return sorted(aliases.values(), key=lambda s: (-s["size"], s["address"], s["name"]))


def make_report(elf, limit, sections, symbols):
    if limit <= 0:
        raise ValueError("Flash limit must be positive")
    used = sum(sections[n]["size"] for n in APP_SECTIONS)
    groups = {}
    for symbol in symbols:
        scope = symbol["namespace"]
        groups[scope] = groups.get(scope, 0) + symbol["size"]
    contributors = [{"namespace": n, "size": size} for n, size in sorted(groups.items(), key=lambda item: (-item[1], item[0]))]
    return {
        "elf": elf.name,
        "elf_sha256": hashlib.sha256(elf.read_bytes()).hexdigest(),
        "flash_used": used,
        "flash_limit": limit,
        "flash_free": limit - used,
        "flash_percent": round(used * 100 / limit, 4),
        "ram_used": sum(sections[n]["size"] for n in RAM_SECTIONS),
        "budget_sections": {n: sections[n] for n in dict.fromkeys(APP_SECTIONS + RAM_SECTIONS)},
        "attribution_note": "Estimates from named symbols in .flash.text/.flash.rodata; exact address/size aliases counted once. "
        "Unnamed data, padding and other budget sections are excluded; partial symbol overlaps may remain.",
        "named_flash_bytes_estimate": sum(s["size"] for s in symbols),
        "largest_namespaces": contributors[:20],
        "largest_symbols": symbols[:20],
    }


def find_tool(name, toolchain_bin):
    directories = [Path(toolchain_bin)] if toolchain_bin else []
    core = Path(os.environ["PLATFORMIO_CORE_DIR"]) if os.environ.get("PLATFORMIO_CORE_DIR") else Path.home() / ".platformio"
    directories.append(core / "packages/toolchain-xtensa-esp-elf/bin")
    for directory in directories:
        candidate = directory / f"xtensa-esp-elf-{name}"
        if candidate.is_file() and os.access(candidate, os.X_OK):
            return str(candidate)
    if found := shutil.which(f"xtensa-esp-elf-{name}"):
        return found
    raise ValueError(f"Cannot find xtensa-esp-elf-{name}; set PLATFORMIO_CORE_DIR, PATH, or --toolchain-bin")


def show_report(report):
    print(f"Flash: {report['flash_used']:,} / {report['flash_limit']:,} bytes ({report['flash_percent']:.2f}%); free {report['flash_free']:,} bytes")
    print(f"RAM: {report['ram_used']:,} bytes (.dram0.data + .dram0.bss)")
    print("Contributor estimates (named flash symbols, aliases counted once):")
    for item in report["largest_namespaces"][:8]:
        print(f"  {item['size']:>10,}  {item['namespace']}")
    print("Largest symbols:")
    for item in report["largest_symbols"][:10]:
        print(f"  {item['size']:>10,}  {item['name']}")
    print(report["attribution_note"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, default=Path(".pio/build/firmware/firmware.elf"))
    parser.add_argument("--limit", type=int, default=8323072)
    parser.add_argument("--toolchain-bin")
    parser.add_argument("--json", nargs="?", const="-", metavar="PATH", help="write JSON to PATH, or stdout when omitted")
    args = parser.parse_args()
    try:
        size = subprocess.check_output([find_tool("size", args.toolchain_bin), "-A", str(args.elf)], text=True)
        sections = parse_sections(size)
        nm = subprocess.check_output([find_tool("nm", args.toolchain_bin), "-S", "-C", "--defined-only", str(args.elf)], text=True)
        report = make_report(args.elf, args.limit, sections, parse_symbols(nm, sections))
        encoded = json.dumps(report, indent=2, sort_keys=True) + "\n"
        if args.json == "-":
            print(encoded, end="")
        else:
            show_report(report)
            if args.json:
                Path(args.json).write_text(encoded, encoding="utf-8")
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.error(str(error))


if __name__ == "__main__":
    main()
