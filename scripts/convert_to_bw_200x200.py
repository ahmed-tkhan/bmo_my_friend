
import os
from PIL import Image
import numpy as np

# --- CONFIG ---
RAW_DIR = os.path.join(os.path.dirname(__file__), 'raw_images')
OUT_DIR = os.path.join(os.path.dirname(__file__), 'processed_pngs')
os.makedirs(OUT_DIR, exist_ok=True)

# Choose dithering method: 'bayer' or 'superpixel'
METHOD = 'bayer'  # or 'superpixel'
SUPERPIXEL_SIZE = 2  # 2x2 blocks for superpixel dithering

# 8x8 Bayer matrix for ordered dithering
BAYER_8x8 = (1/64) * np.array([
    [ 0, 48, 12, 60,  3, 51, 15, 63],
    [32, 16, 44, 28, 35, 19, 47, 31],
    [ 8, 56,  4, 52, 11, 59,  7, 55],
    [40, 24, 36, 20, 43, 27, 39, 23],
    [ 2, 50, 14, 62,  1, 49, 13, 61],
    [34, 18, 46, 30, 33, 17, 45, 29],
    [10, 58,  6, 54,  9, 57,  5, 53],
    [42, 26, 38, 22, 41, 25, 37, 21],
])

def bayer_dither(img):
    arr = np.array(img, dtype=np.float32) / 255.0
    h, w = arr.shape
    tiled = np.tile(BAYER_8x8, (h // 8 + 1, w // 8 + 1))[:h, :w]
    bw = (arr > tiled).astype(np.uint8) * 255
    return Image.fromarray(bw, mode='L').convert('1')

def superpixel_dither(img, block=2):
    arr = np.array(img, dtype=np.float32) / 255.0
    h, w = arr.shape
    out = np.ones((h, w), dtype=np.uint8) * 255
    for y in range(0, h, block):
        for x in range(0, w, block):
            region = arr[y:y+block, x:x+block]
            mean = np.mean(region)
            n_pixels = region.size
            n_black = int(round((1 - mean) * n_pixels))
            flat_idx = np.argsort(region, axis=None)[:n_black]
            region_out = np.ones(region.shape, dtype=np.uint8) * 255
            np.put(region_out, flat_idx, 0)
            out[y:y+block, x:x+block] = region_out
    return Image.fromarray(out, mode='L').convert('1')

def process_image(filename):
    in_path = os.path.join(RAW_DIR, filename)
    out_path = os.path.join(OUT_DIR, filename)
    img = Image.open(in_path).convert('L')  # Grayscale
    # Resize keeping aspect ratio, max 200x200
    img.thumbnail((200, 200), Image.LANCZOS)
    # Center on 200x200 canvas
    canvas = Image.new('L', (200, 200), 255)
    x = (200 - img.width) // 2
    y = (200 - img.height) // 2
    canvas.paste(img, (x, y))
    # Dither
    if METHOD == 'bayer':
        bw = bayer_dither(canvas)
    elif METHOD == 'superpixel':
        bw = superpixel_dither(canvas, block=SUPERPIXEL_SIZE)
    else:
        raise ValueError('Unknown dithering method')
    bw.save(out_path)
    print(f"Saved: {out_path}")

def main():
    for fname in os.listdir(RAW_DIR):
        if fname.lower().endswith('.png'):
            process_image(fname)

if __name__ == '__main__':
    main()
