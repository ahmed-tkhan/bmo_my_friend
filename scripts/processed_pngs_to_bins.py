"""
Convert 200x200 monochrome PNGs in scripts/processed_pngs to raw 5000-byte .bin files in data/
Each output file is exactly 5000 bytes (200*200/8) with MSB-first packing per row.
Usage: python processed_pngs_to_bins.py
"""
from PIL import Image
import os

IN_DIR = os.path.join(os.path.dirname(__file__), 'processed_pngs')
OUT_DIR = os.path.join(os.path.dirname(__file__), '..', 'data')
os.makedirs(OUT_DIR, exist_ok=True)

def pack_image(img: Image.Image):
    # Flip image vertically (upside-down) to match display orientation
    img = img.transpose(Image.FLIP_TOP_BOTTOM)
    img = img.convert('L')
    # Create 200x200 canvas and paste with requested offset: up 10 px, right 5 px
    canvas = Image.new('L', (200, 200), 255)
    base_x = (200 - img.width) // 2
    base_y = (200 - img.height) // 2
    # Apply user-requested shift: right by 5, up by 10
    x = base_x + 50
    y = base_y - 100
    # Clamp coordinates so paste stays within canvas
    if x < 0:
        x = 0
    if y < 0:
        y = 0
    if x > 200 - img.width:
        x = 200 - img.width
    if y > 200 - img.height:
        y = 200 - img.height
    canvas.paste(img, (x, y))
    bw = canvas.convert('1')  # 1-bit
    pixels = bw.load()
    out = bytearray()
    for y in range(200):
        for byte_x in range(0, 200, 8):
            b = 0
            for bit in range(8):
                px = pixels[byte_x + bit, y]
                # PIL '1' mode: pixel is 0 for black, 255 for white
                is_black = (px == 0)
                # Display expects byte format where 1 bits == white, 0 bits == black
                bit_val = 0 if is_black else 1
                b = (b << 1) | bit_val
            out.append(b)
    return bytes(out)


def main():
    files = [f for f in os.listdir(IN_DIR) if f.lower().endswith('.png')]
    if not files:
        print('No PNGs found in', IN_DIR)
        return
    for fname in files:
        path = os.path.join(IN_DIR, fname)
        try:
            img = Image.open(path)
        except Exception as e:
            print('Failed to open', path, e)
            continue
        data = pack_image(img)
        if len(data) != 5000:
            print('Packed size unexpected for', fname, len(data))
            continue
        out_name = os.path.splitext(fname)[0] + '.bin'
        out_path = os.path.join(OUT_DIR, out_name)
        with open(out_path, 'wb') as f:
            f.write(data)
        print('Wrote', out_path)

if __name__ == '__main__':
    main()
