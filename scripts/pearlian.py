#!/usr/bin/env python3
import argparse, math, random

W, H = 200, 200  # output size
BYTES = (W * H) // 4  # 10,000 bytes

def fade(t): return t*t*t*(t*(t*6 - 15) + 10)
def lerp(a, b, t): return a + t * (b - a)

class Perlin2D:
    def __init__(self, seed=0):
        rng = random.Random(seed)
        p = list(range(256))
        rng.shuffle(p)
        self.p = p + p  # wrap

    def grad(self, h, x, y):
        # 8 gradient directions from low 3 bits
        g = h & 7
        if g == 0:  return  x + y
        if g == 1:  return  x - y
        if g == 2:  return -x + y
        if g == 3:  return -x - y
        if g == 4:  return  x
        if g == 5:  return -x
        if g == 6:  return  y
        return -y

    def noise(self, x, y):
        X = math.floor(x) & 255
        Y = math.floor(y) & 255
        xf = x - math.floor(x)
        yf = y - math.floor(y)

        u = fade(xf)
        v = fade(yf)

        A  = self.p[X]     + Y
        AA = self.p[A]
        AB = self.p[A + 1]
        B  = self.p[X + 1] + Y
        BA = self.p[B]
        BB = self.p[B + 1]

        n00 = self.grad(self.p[AA], xf,     yf)
        n10 = self.grad(self.p[BA], xf - 1, yf)
        n01 = self.grad(self.p[AB], xf,     yf - 1)
        n11 = self.grad(self.p[BB], xf - 1, yf - 1)

        x1 = lerp(n00, n10, u)
        x2 = lerp(n01, n11, u)
        n  = lerp(x1, x2, v)            # approx in [-1,1]
        return 0.5 * (n + 1.0)          # map to [0,1]

def fbm(perlin, x, y, octaves=4, lacunarity=2.0, gain=0.5, scale=40.0):
    # Fractional Brownian Motion
    val = 0.0
    amp = 1.0
    freq = 1.0
    norm = 0.0
    for _ in range(octaves):
        val += amp * perlin.noise((x * freq) / scale, (y * freq) / scale)
        norm += amp
        amp *= gain
        freq *= lacunarity
    return val / norm if norm != 0 else 0.0

def quantize_to_2bpp(v):
    # v in [0,1] -> 0..3 (0=white .. 3=black)
    q = int(v * 4.0)
    if q < 0: q = 0
    if q > 3: q = 3
    return q

def pack_4px(byte_val, lv, idx_in_byte):
    # idx_in_byte: 0..3 -> place at bits 7..6, 5..4, 3..2, 1..0
    shift = (3 - idx_in_byte) * 2
    byte_val &= ~(0x3 << shift)
    byte_val |= (lv & 0x3) << shift
    return byte_val

def main():
    ap = argparse.ArgumentParser(description="Generate 200x200 2-bpp Perlin noise RAW (10,000 bytes).")
    ap.add_argument("--out", default="noise_200x200_2bpp.bin", help="Output filename")
    ap.add_argument("--seed", type=int, default=1337, help="Random seed")
    ap.add_argument("--scale", type=float, default=40.0, help="Feature scale (bigger = smoother)")
    ap.add_argument("--octaves", type=int, default=4, help="Number of FBM octaves")
    ap.add_argument("--gain", type=float, default=0.5, help="Amplitude gain per octave")
    ap.add_argument("--lacunarity", type=float, default=2.0, help="Frequency multiplier per octave")
    args = ap.parse_args()

    perlin = Perlin2D(seed=args.seed)
    buf = bytearray(BYTES)

    for y in range(H):
        for x in range(W):
            v = fbm(perlin, x, y, octaves=args.octaves, lacunarity=args.lacunarity, gain=args.gain, scale=args.scale)
            # Optional gamma to bias contrast:
            # v = pow(v, 1.0)  # 1.0 = linear
            lv = quantize_to_2bpp(v)
            pix_index = y * W + x
            byte_index = pix_index >> 2
            idx_in_byte = pix_index & 3
            buf[byte_index] = pack_4px(buf[byte_index], lv, idx_in_byte)

    with open(args.out, "wb") as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {args.out}")

if __name__ == "__main__":
    main()