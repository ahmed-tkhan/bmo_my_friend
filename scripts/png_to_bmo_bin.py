#!/usr/bin/env python3
"""
Convert PNG images to custom BMO .bin format for partial refresh display.
- Resizes to 100x100, centers on 200x200 canvas
- Applies ordered dithering for black & white
- Prepends 4 integers (x, y, width, height) as region info
- Output: data/{name}.bin
"""
import struct


import os
import struct
from PIL import Image

IN_DIR = os.path.join(os.path.dirname(__file__), 'region_pngs')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'data')
os.makedirs(OUT_DIR, exist_ok=True)

def parse_filename(filename):
    # Expects: name_x_y_w_h.png
    base = os.path.splitext(filename)[0]
    parts = base.split('_')
    if len(parts) < 5:
        raise ValueError(f"Filename {filename} does not contain region info.")
    x, y, w, h = map(int, parts[-4:])
    return x, y, w, h

def pack_1bpp(img):
    # img: PIL Image, mode '1' or 'L', size (w, h)
    w, h = img.size
    # Convert to grayscale, then threshold to 0/1
    arr = img.convert('L')
    arr = arr.point(lambda x: 1 if x < 128 else 0, '1')
    arr = arr.convert('L')
    flat = list(arr.getdata())  # 0 (black) or 1 (white)
    packed = bytearray()
    for i in range(0, len(flat), 8):
        byte = 0
        for bit in range(8):
            if i + bit < len(flat):
                # Invert: 0 (black) -> 1, 1 (white) -> 0
                byte = (byte << 1) | (1 if flat[i + bit] == 0 else 0)
            else:
                byte = (byte << 1)
        packed.append(byte)
    return bytes(packed)

def process_image(filename):
    in_path = os.path.join(IN_DIR, filename)
    x, y, w, h = parse_filename(filename)
    img = Image.open(in_path).convert('1')
    if img.size != (w, h):
        img = img.resize((w, h), Image.NEAREST)
    data = pack_1bpp(img)
    header = struct.pack('<4i', x, y, w, h)
    out_name = os.path.splitext(filename)[0] + '.bin'
    out_path = os.path.join(OUT_DIR, out_name)
    with open(out_path, 'wb') as f:
        f.write(header)
        f.write(data)
    print(f"Saved: {out_path}")

def main():
    for fname in os.listdir(IN_DIR):
        if fname.lower().endswith('.png'):
            process_image(fname)

if __name__ == '__main__':
    main()
