#!/usr/bin/env python3
"""
Display a custom BMO .bin file (with region header) as an image, showing the region rectangle.
"""
import struct

import os
import struct
from PIL import Image, ImageDraw

def read_bin(filepath):
    with open(filepath, 'rb') as f:
        header = f.read(16)
        x, y, w, h = struct.unpack('<4i', header)
        data = f.read()
    return x, y, w, h, data

def unpack_1bpp(data, w, h):
    img = Image.new('1', (w, h), 1)
    pixels = img.load()
    byte_idx = 0
    bit_idx = 7
    for y in range(h):
        for x in range(w):
            if byte_idx >= len(data):
                break
            bit = (data[byte_idx] >> bit_idx) & 1
            pixels[x, y] = 0 if bit else 1
            bit_idx -= 1
            if bit_idx < 0:
                bit_idx = 7
                byte_idx += 1
    return img

def display_bin(filepath, outdir):
    x, y, w, h, data = read_bin(filepath)
    region = unpack_1bpp(data, w, h)
    canvas = Image.new('1', (200, 200), 1)
    canvas.paste(region, (x, y))
    draw = ImageDraw.Draw(canvas)
    draw.rectangle([x, y, x + w - 1, y + h - 1], outline=0)
    base = os.path.splitext(os.path.basename(filepath))[0]
    out_path = os.path.join(outdir, base + '_sim.png')
    canvas.save(out_path)
    print(f"Saved: {out_path}")

def main():
    data_dir = os.path.join(os.path.dirname(__file__), '..', 'data')
    out_dir = os.path.join(os.path.dirname(__file__), 'simulated_faces')
    os.makedirs(out_dir, exist_ok=True)
    for fname in os.listdir(data_dir):
        if fname.lower().endswith('.bin'):
            display_bin(os.path.join(data_dir, fname), out_dir)

if __name__ == '__main__':
    main()
