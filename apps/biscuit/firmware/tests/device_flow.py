#!/usr/bin/env python3
"""Exercise complete game flows through LVGL taps; restore progress afterward.
Run with PlatformIO's Python: python firmware/tests/device_flow.py --port PORT
"""

import argparse
import importlib.util
import json
import re
import time
from pathlib import Path

spec = importlib.util.spec_from_file_location("device", Path(__file__).parents[1] / "tools/device.py")
assert spec and spec.loader, "tools/device.py not found"
device = importlib.util.module_from_spec(spec)
spec.loader.exec_module(device)


def run(port, output):
    link = device.open_connection(device.choose_port(port))
    output.mkdir(parents=True, exist_ok=True)

    def send(command):
        link.reset_input_buffer()
        link.write((command + "\n").encode())
        return time.monotonic() + 8

    def status():
        deadline = send("STATUS")
        return json.loads(device.expect_line(link, "STATUS ", deadline)[7:])

    def tree():
        deadline = send("TREE")
        items = []
        while True:
            line = device.read_line(link, deadline)
            if line == "END_TREE":
                return items
            match = re.match(r"UI (button|text) (\d+) (\d+) (\d+) (\d+) (enabled|disabled) ?(.*)", line)
            if match:
                kind, x1, y1, x2, y2, state, text = match.groups()
                items.append(dict(kind=kind, box=tuple(map(int, (x1, y1, x2, y2))), enabled=state == "enabled", text=text))

    def target(text):
        items = tree()
        labels = [i for i in items if i["kind"] == "text" and i["text"] == text]
        if not labels:
            labels = [i for i in items if i["kind"] == "text" and text in i["text"]]
        hits = []
        for label in labels:
            x1, y1, x2, y2 = label["box"]
            x, y = (x1 + x2) // 2, (y1 + y2) // 2
            for b in items:
                left, top, right, bottom = b["box"]
                if b["kind"] == "button" and b["enabled"] and left <= x <= right and top <= y <= bottom:
                    hits.append(((left + right) // 2, (top + bottom) // 2))
        if len(hits) != 1:
            raise AssertionError(f"Expected one enabled button for {text!r}; found {hits}; UI={items}")
        return hits[0]

    def tap(x, y):
        deadline = send(f"TAP {x} {y}")
        device.expect_line(link, "OK TAP", deadline)
        time.sleep(0.28)

    def click(text):
        tap(*target(text))

    def has(text):
        return any(i["text"] == text for i in tree())

    def capture(name):
        send("SHOT")
        rgb = device.read_frame(link, time.monotonic() + 90)
        (output / f"{name}.png").write_bytes(device.png_bytes(rgb))
        return rgb

    def require_detail(rgb, name, box, minimum=2):
        left, top, right, bottom = box
        colors = {rgb[(y * device.WIDTH + x) * 3 : (y * device.WIDTH + x) * 3 + 3] for y in range(top, bottom) for x in range(left, right)}
        assert len(colors) >= minimum, f"{name} is blank or visually flat ({len(colors)} colors)"

    def home():
        for _ in range(6):
            if status()["view"] == 0:
                return
            click("Back" if has("Back") else "<")
        raise AssertionError("Cannot get home through Back")

    def read_story(choice):
        click("Read")
        click("The Moon Biscuit")
        for _ in range(50):
            if has("Choose"):
                click("Choose")
                break
            click("Next")
        else:
            raise AssertionError("Opening never reached its choice")
        click(choice)
        for _ in range(30):
            if has("The end"):
                click("The end")
                return
            click("Next")
        raise AssertionError("Ending never finished")

    before = status()
    assert before["view"] == 0 and not before["testing"], before
    deadline = send("TEST BEGIN")
    device.expect_line(link, "OK TEST BEGIN", deadline)
    try:
        if status()["sleeping"]:
            click("Wake")
        print("Checking care and both story endings...", flush=True)
        home_rgb = capture("home")
        for index, left in enumerate((72, 188, 304)):
            require_detail(home_rgb, f"need icon {index}", (left + 6, 96, left + 18, 108))
            require_detail(home_rgb, f"need meter {index}", (left + 23, 100, left + 51, 107))
        for index, left in enumerate((96, 169, 242, 315)):
            require_detail(home_rgb, f"home action icon {index}", (left + 22, 362, left + 46, 386))
        click("Feed")
        fed = status()["friendship"]
        click("Feed")
        assert status()["friendship"] == fed, "Repeated feeding farmed daily friendship"
        tap(240, 230)
        click("Read")
        click("The Moon Biscuit")
        capture("story")
        home()
        read_story("Follow the wire upstairs")
        expected = before["stars"] + (0 if before["stories"] & 1 else 3)
        assert status()["stars"] == expected
        read_story("Open the cabinet's base")
        assert status()["stars"] == expected, "The other ending duplicated story stars"

        click("Read")
        print("Checking illustrated discoveries and notebook...", flush=True)
        click("Discoveries")
        capture("discoveries")
        click("Topics")
        click("Space")
        items = tree()
        # Select the first fact card by its button geometry, not any model API.
        card = next(i for i in items if i["kind"] == "button" and i["box"][1] == 210)
        x1, y1, x2, y2 = card["box"]
        tap((x1 + x2) // 2, (y1 + y2) // 2)
        capture("discovery-cover")
        click("Let's find out")
        capture("discovery-reading")
        for _ in range(15):
            if has("Keep"):
                break
            click("Next")
        else:
            raise AssertionError("Discovery never reached Keep")
        click("Source")
        capture("source")
        click("Back to our book")
        click("Keep")
        assert status()["discoveries"] >= before["discoveries"]
        home()

        print("Checking fetch and trick training...", flush=True)
        click("Play")
        for _ in range(5):
            click("Ball")
        assert status()["daily"] & 2, "Fetch did not record play"
        click("More")
        click("Learn tricks")
        progress = status()["tricks"][0]
        for _ in range(3 - progress):
            click("Sit")
            pattern = next(i["text"] for i in tree() if " - " in i["text"]).split(" - ")
            click("My turn")
            for cue in pattern:
                click(cue)
            click("More")
            click("Learn tricks")
        click("Sit")
        assert status()["tricks"][0] == 3 and status()["daily"] & 16
        capture("trick")
        print("Checking daily activities, sleep and settings...", flush=True)
        click("More")
        click("Today's adventure")
        capture("today")
        home()
        click("More")
        click("Cozy nap")
        assert status()["sleeping"]
        click("Wake")
        assert not status()["sleeping"]
        click("More")
        click("Settings")
        capture("settings")
        click("Set date & time")
        capture("clock")
        click("Save time")
        home()
        results = status()
        print(json.dumps({"before": before, "exercised": results, "captures": str(output)}, indent=2))
    finally:
        deadline = send("TEST END")
        device.expect_line(link, "OK TEST END", deadline)
        restored = status()
        for key in ("stars", "stories", "discoveries", "days", "friendship", "tricks", "sleeping", "daily", "claimed", "stickers", "brightness"):
            assert restored[key] == before[key], f"Test failed to restore {key}"
        assert not restored["testing"]
        link.close()
    print("PASS: care, both story endings, discoveries/source/notebook, fetch, training, daily, sleep, settings, and progress restoration.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port")
    parser.add_argument("--output", type=Path, default=Path("output/device-qa"))
    args = parser.parse_args()
    run(args.port, args.output)
