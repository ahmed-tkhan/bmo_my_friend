import os
from PIL import Image
import numpy as np

IN_DIR = os.path.join(os.path.dirname(__file__), 'processed_pngs')
OUT_DIR = os.path.join(os.path.dirname(__file__), 'region_pngs')
os.makedirs(OUT_DIR, exist_ok=True)


def find_bounding_box(img):
    arr = np.array(img)
    if img.mode == '1':
        arr = arr == 0
    else:
        arr = arr < 128
    coords = np.argwhere(arr)
    if coords.size == 0:
        return None
    y0, x0 = coords.min(axis=0)
    y1, x1 = coords.max(axis=0) + 1
    return x0, y0, x1 - x0, y1 - y0


# --- Find max bounding box for all images ---
def get_max_region():
    max_w, max_h = 0, 0
    for fname in os.listdir(IN_DIR):
        if fname.lower().endswith('.png'):
            img = Image.open(os.path.join(IN_DIR, fname)).convert('1')
            bbox = find_bounding_box(img)
            if bbox is not None:
                _, _, w, h = bbox
                if w > max_w:
                    max_w = w
                if h > max_h:
                    max_h = h
    # Pad width to next multiple of 8
    max_w = ((max_w + 7) // 8) * 8
    return max_w, max_h

def process_image(filename, region_w, region_h):
    in_path = os.path.join(IN_DIR, filename)
    img = Image.open(in_path).convert('1')
    # Center the region on the 200x200 canvas
    x = (200 - region_w) // 2
    y = (200 - region_h) // 2
    # Crop the original image's bounding box, paste into region
    bbox = find_bounding_box(img)
    region = Image.new('1', (region_w, region_h), 1)
    if bbox is not None:
        bx, by, bw, bh = bbox
        crop = img.crop((bx, by, bx + bw, by + bh))
        # Center the cropped content in the region
        rx = (region_w - bw) // 2
        ry = (region_h - bh) // 2
        region.paste(crop, (rx, ry))
    base, ext = os.path.splitext(filename)
    out_name = f"{base}_{x}_{y}_{region_w}_{region_h}.png"
    out_path = os.path.join(OUT_DIR, out_name)
    region.save(out_path)
    print(f"Saved: {out_path}")

def main():
    region_w, region_h = get_max_region()
    x = (200 - region_w) // 2
    y = (200 - region_h) // 2
    print(f"Using region: x={x}, y={y}, w={region_w}, h={region_h}")
    for fname in os.listdir(IN_DIR):
        if fname.lower().endswith('.png'):
            process_image(fname, region_w, region_h)

if __name__ == '__main__':
    main()
