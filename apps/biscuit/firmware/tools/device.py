#!/usr/bin/env python3
"""Biscuit USB diagnostics; screenshots are LCD framebuffer captures.

Use the PlatformIO Python environment (which includes pyserial), for example:
    python firmware/tools/device.py --port /dev/cu.usbmodem123 status
    python firmware/tools/device.py shot screen.png
Self-tests need only the Python standard library and never open a port.
"""

import argparse
import glob
import json
import struct
import sys
import time
import zlib
from pathlib import Path


WIDTH = HEIGHT = 480
PIXELS = WIDTH * HEIGHT


def read_exact(connection, size, deadline):
    data = bytearray()
    while len(data) < size:
        if time.monotonic() >= deadline:
            raise TimeoutError(f"Device response timed out ({len(data)}/{size} bytes)")
        chunk = connection.read(size - len(data))
        if chunk:
            data.extend(chunk)
    return bytes(data)


def read_line(connection, deadline):
    line = bytearray()
    while len(line) <= 65536:
        byte = read_exact(connection, 1, deadline)
        if byte == b"\n":
            text = line.rstrip(b"\r").decode("utf-8", errors="replace")
            if text.startswith(("ERROR ", "FATAL ")):
                raise ValueError(text)
            return text
        line.extend(byte)
    raise ValueError("Device response line is too long")


def expect_line(connection, prefix, deadline):
    while True:
        line = read_line(connection, deadline)
        if line.startswith(prefix):
            return line


def decode_frame(payload, runs):
    """Validate little-endian (run length, RGB565) pairs and return RGB bytes."""
    if not 1 <= runs <= PIXELS or len(payload) != runs * 4:
        raise ValueError("Invalid framebuffer run count or payload length")
    pixels = 0
    rgb = bytearray()
    for count, color in struct.iter_unpack("<HH", payload):
        if count == 0 or pixels + count > PIXELS:
            raise ValueError("Invalid framebuffer run length")
        red = ((color >> 11) * 255 + 15) // 31
        green = (((color >> 5) & 63) * 255 + 31) // 63
        blue = ((color & 31) * 255 + 15) // 31
        rgb.extend(bytes((red, green, blue)) * count)
        pixels += count
    if pixels != PIXELS:
        raise ValueError(f"Incomplete framebuffer: {pixels}/{PIXELS} pixels")
    return bytes(rgb)


def read_frame(connection, deadline):
    header = expect_line(connection, "FRAME ", deadline)
    try:
        runs = int(header[6:])
    except ValueError as error:
        raise ValueError("Invalid FRAME header") from error
    if not 1 <= runs <= PIXELS:
        raise ValueError("Invalid framebuffer run count")
    payload = read_exact(connection, runs * 4, deadline)
    # The firmware emits a separating newline before END_FRAME. Binary data
    # above is never line-decoded: either field can contain a newline byte.
    terminator = read_line(connection, deadline)
    if terminator == "":
        terminator = read_line(connection, deadline)
    if terminator != "END_FRAME":
        raise ValueError("Missing END_FRAME terminator")
    return decode_frame(payload, runs)


def png_bytes(rgb, round_mask=True):
    """Encode the exact 480x480 pixels, optionally hiding the panel's corners."""
    if len(rgb) != PIXELS * 3:
        raise ValueError("Expected a complete 480x480 RGB image")

    def chunk(kind, data):
        checksum = zlib.crc32(kind + data) & 0xFFFFFFFF
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", checksum)

    raw = bytearray()
    for y in range(HEIGHT):
        raw.append(0)  # PNG filter: none.
        row = rgb[y * WIDTH * 3 : (y + 1) * WIDTH * 3]
        if round_mask:
            for x in range(WIDTH):
                raw.extend(row[x * 3 : x * 3 + 3])
                # Pixel centers measured against a circle centered at (240,240).
                inside = (2 * x + 1 - WIDTH) ** 2 + (2 * y + 1 - HEIGHT) ** 2 <= WIDTH**2
                raw.append(255 if inside else 0)
        else:
            raw.extend(row)
    header = struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 6 if round_mask else 2, 0, 0, 0)
    return b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", header) + chunk(b"IDAT", zlib.compress(raw)) + chunk(b"IEND", b"")


def choose_port(explicit):
    if explicit:
        return explicit
    ports = sorted(glob.glob("/dev/cu.usbmodem*"))
    if len(ports) != 1:
        found = ", ".join(ports) if ports else "none"
        raise ValueError(f"Use --port: expected exactly one /dev/cu.usbmodem* device, found {found}")
    return ports[0]


def open_connection(port):
    """Open without pyserial's sequential DTR/RTS transitions (POSIX only).

    A USB driver can still change lines during the OS open itself; this avoids
    the additional reset pulse caused by pyserial on the board's CH343 bridge.
    """
    try:
        import serial
    except ImportError as error:
        raise RuntimeError("pyserial is required; run this with the PlatformIO Python environment") from error
    try:
        import fcntl
        import termios
    except ImportError as error:
        raise RuntimeError("Atomic DTR/RTS setup requires a POSIX serial backend") from error

    class AtomicOpenSerial(serial.Serial):
        # Serial.open() calls these while is_open is still False. Its separate
        # writes can pass through (DTR=False, RTS=True), pulling ESP32 EN low.
        # Explicit line changes after open retain pyserial's normal behavior.
        def _update_dtr_state(self):
            if self.is_open:
                super()._update_dtr_state()

        def _update_rts_state(self):
            if self.is_open:
                super()._update_rts_state()

    connection = AtomicOpenSerial(port=None, baudrate=115200, timeout=0.1, write_timeout=1)
    connection.dtr = False
    connection.rts = False
    connection.port = port
    try:
        connection.open()
        fd = connection.fileno()
        attributes = termios.tcgetattr(fd)
        if attributes[2] & termios.HUPCL:
            attributes[2] &= ~termios.HUPCL
            termios.tcsetattr(fd, termios.TCSANOW, attributes)
        lines = struct.unpack("I", fcntl.ioctl(fd, termios.TIOCMGET, struct.pack("I", 0)))[0]
        lines &= ~(termios.TIOCM_DTR | termios.TIOCM_RTS)
        fcntl.ioctl(fd, termios.TIOCMSET, struct.pack("I", lines))
    except BaseException:
        connection.close()
        raise
    return connection


def run(args):
    connection = open_connection(choose_port(args.port))
    try:
        connection.reset_input_buffer()
        command = {
            "status": "STATUS",
            "tree": "TREE",
            "shot": "SHOT",
            "test-begin": "TEST BEGIN",
            "test-end": "TEST END",
            "save": "SAVE",
        }.get(args.command)
        if args.command == "tap":
            command = f"TAP {args.x} {args.y}"
        elif args.command == "time":
            command = f"TIME {int(time.time())}"
        deadline = time.monotonic() + (90 if args.command == "shot" else 5)
        written = connection.write((command + "\n").encode("ascii"))
        if written != len(command) + 1:
            raise OSError("Incomplete serial command write")

        if args.command == "shot":
            rgb = read_frame(connection, deadline)
            output = Path(args.output)
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_bytes(png_bytes(rgb, not args.square))
            print(f"Framebuffer capture: {args.output} (480x480)")
        elif args.command in ("status", "time", "save"):
            line = expect_line(connection, "STATUS ", deadline)
            data = json.loads(line[7:])
            if not isinstance(data, dict):
                raise ValueError("Expected a STATUS JSON object")
            print(json.dumps(data, indent=2))
        elif args.command == "tree":
            lines = []
            size = 0
            while True:
                line = read_line(connection, deadline)
                if line == "END_TREE":
                    break
                size += len(line)
                if size > 1024 * 1024:
                    raise ValueError("Device UI tree is too large")
                lines.append(line)
            print("\n".join(lines))
        else:
            print(expect_line(connection, "OK TAP" if args.command == "tap" else "OK " + command, deadline))
            if args.command == "tap":
                # ACK queues a 120 ms touch; leave time for release and LVGL's
                # click callback before the caller requests its next frame.
                time.sleep(0.3)
    finally:
        connection.close()


def self_test_modem_lines():
    """Exercise real pyserial with mocked OS calls; never open a device."""
    try:
        import serial.serialposix as backend
        import termios
    except ImportError:
        return  # The image/protocol self-tests still run without pyserial.
    from unittest.mock import patch

    fd = 424242
    lines = termios.TIOCM_DTR | termios.TIOCM_RTS | termios.TIOCM_CAR
    states, requests, closed, attributes_written = [], [], [], []
    real_close = backend.os.close

    def ioctl(target, request, argument):
        nonlocal lines
        assert target == fd
        requests.append(request)
        if request == termios.TIOCMGET:
            return struct.pack("I", lines)
        value = struct.unpack("I", argument)[0]
        if request == termios.TIOCMBIC:
            lines &= ~value
        elif request == termios.TIOCMBIS:
            lines |= value
        elif request == termios.TIOCMSET:
            lines = value
        else:
            raise AssertionError(f"Unexpected modem request: {request}")
        states.append((bool(lines & termios.TIOCM_DTR), bool(lines & termios.TIOCM_RTS)))
        return 0

    def close(target):
        if target == fd:
            closed.append(target)
        else:
            real_close(target)  # pyserial creates real cancellation pipes.

    attributes = [0, 0, termios.HUPCL | termios.CS8, 0, termios.B115200, termios.B115200, [0] * 32]
    with (
        patch.object(backend.os, "open", return_value=fd),
        patch.object(backend.os, "close", side_effect=close),
        patch.object(backend.termios, "tcgetattr", return_value=attributes),
        patch.object(backend.termios, "tcsetattr", side_effect=lambda fd, when, attr: attributes_written.append(attr[:])),
        patch.object(backend.termios, "tcflush"),
        patch.object(backend.fcntl, "ioctl", side_effect=ioctl),
    ):
        connection = open_connection("mock-device")
        assert connection.is_open and connection.dtr is False and connection.rts is False
        connection.close()
    assert states == [(False, False)], f"Reset-producing modem transitions: {states}"
    assert requests == [termios.TIOCMGET, termios.TIOCMSET]
    assert lines & termios.TIOCM_CAR  # Unrelated modem bits are preserved.
    assert not attributes_written[-1][2] & termios.HUPCL
    assert closed == [fd]
    print("Self-test passed: atomic serial open, HUPCL, modem bits, cleanup (mocked OS)")


def self_test():
    self_test_modem_lines()
    from unittest.mock import Mock, patch

    response = iter(bytes([byte]) for byte in b"OK TAP\r\n")
    connection = Mock()
    connection.read.side_effect = lambda size: next(response)
    connection.write.side_effect = len
    with patch(__name__ + ".open_connection", return_value=connection), patch("builtins.print"), patch.object(time, "sleep"):
        run(argparse.Namespace(port="mock-device", command="tap", x=240, y=240))
    connection.write.assert_called_once_with(b"TAP 240 240\n")
    connection.close.assert_called_once()

    def rejected(call):
        try:
            call()
        except ValueError:
            return
        raise AssertionError("Expected invalid frame to be rejected")

    # The full frame exceeds a uint16 run; include every channel and an interior
    # color to verify RGB565 expansion, not just all-white/all-black endpoints.
    payload = b"".join(struct.pack("<HH", count, color) for count, color in ((65535, 0xF800), (65535, 0x07E0), (65535, 0x001F), (33795, 0x8410)))
    rgb = decode_frame(payload, 4)
    assert len(rgb) == PIXELS * 3
    assert rgb[:3] == bytes((255, 0, 0))
    assert rgb[65535 * 3 : 65535 * 3 + 3] == bytes((0, 255, 0))
    assert rgb[-3:] == bytes((132, 130, 132))
    rejected(lambda: decode_frame(struct.pack("<HH", 0, 0), 1))
    rejected(lambda: decode_frame(struct.pack("<HH", 1, 0), 1))
    rejected(lambda: decode_frame(struct.pack("<HH", 65535, 0) * 4, 4))
    rejected(lambda: decode_frame(payload[:-1], 4))
    rejected(lambda: decode_frame(b"", PIXELS + 1))

    class FragmentedInput:
        def __init__(self, data):
            self.data = data
            self.calls = 0

        def read(self, size):
            self.calls += 1
            if self.calls % 3 == 0:
                return b""  # A temporary empty read must not end the frame.
            size = min(size, 3)
            result, self.data = self.data[:size], self.data[size:]
            return result

    framed = b"boot log\nFRAME 4\n" + payload + b"\nEND_FRAME\r\n"
    assert read_frame(FragmentedInput(framed), time.monotonic() + 1) == rgb
    rejected(lambda: read_frame(FragmentedInput(framed[:-11] + b"WRONG\n"), time.monotonic() + 1))
    for mask in (False, True):
        png = png_bytes(rgb, mask)
        assert png[:8] == b"\x89PNG\r\n\x1a\n"
        offset, compressed = 8, b""
        while offset < len(png):
            size = struct.unpack_from(">I", png, offset)[0]
            kind = png[offset + 4 : offset + 8]
            data = png[offset + 8 : offset + 8 + size]
            crc = struct.unpack_from(">I", png, offset + 8 + size)[0]
            assert crc == zlib.crc32(kind + data) & 0xFFFFFFFF
            if kind == b"IDAT":
                compressed += data
            offset += size + 12
        raw = zlib.decompress(compressed)
        stride = WIDTH * (4 if mask else 3) + 1
        assert len(raw) == HEIGHT * stride
        if mask:
            assert raw[4] == 0
            assert raw[240 * stride + 1 + 240 * 4 + 3] == 255
        else:
            assert raw[1:4] == rgb[:3]
    print("Self-test passed: frame validation, fragmented reads, RGB565, PNG, circle mask")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", help="serial port; default: the sole /dev/cu.usbmodem* device")
    parser.add_argument("--self-test", action="store_true", help="run offline parser/PNG tests; no serial access")
    commands = parser.add_subparsers(dest="command")
    for command in ("status", "tree", "time", "test-begin", "test-end", "save"):
        commands.add_parser(command)
    tap = commands.add_parser("tap", help="inject a touch through the LVGL input driver")
    tap.add_argument("x", type=int, choices=range(WIDTH), metavar="X")
    tap.add_argument("y", type=int, choices=range(HEIGHT), metavar="Y")
    shot = commands.add_parser("shot", help="capture the LCD framebuffer as a PNG")
    shot.add_argument("output")
    shot.add_argument("--square", action="store_true", help="keep pixels outside the visible round panel")
    args = parser.parse_args()
    if args.self_test:
        if args.command:
            parser.error("--self-test cannot be combined with a device command")
        self_test()
        return 0
    if not args.command:
        parser.error("choose a command or --self-test")
    try:
        run(args)
        return 0
    except (OSError, ValueError, RuntimeError) as error:
        print(f"device.py: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
