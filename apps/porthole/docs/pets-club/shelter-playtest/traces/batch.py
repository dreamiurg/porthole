import sys
import subprocess
import shutil
from PIL import Image

# batch.py BASE OUTPNG name1 "lines" name2 "lines" ...
base, outpng, args = sys.argv[1], sys.argv[2], sys.argv[3:]
names = []
for n, lines in zip(args[::2], args[1::2], strict=True):
    shutil.copy(f"t/{base}.txt", f"t/p_{n}.txt")
    open(f"t/p_{n}.txt", "a").write(lines.replace(";", "\n") + "\nshot r\n")
    import os

    subprocess.run(["python3", "drive.py", f"t/p_{n}.txt", f"out_{n}"], check=True, env=dict(os.environ, WD=os.path.abspath(f"wd_{n}")))
    names.append(n)
s = Image.new("RGB", (480 * min(4, len(names)), 480 * ((len(names) + 3) // 4)))
for i, n in enumerate(names):
    s.paste(Image.open(f"out_{n}/r.png"), (480 * (i % 4), 480 * (i // 4)))
s.save(outpng)
