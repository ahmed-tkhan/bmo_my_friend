#!/usr/bin/env python3
"""Render region .bin files (in data/regions) using existing display_bmo_bin utilities."""
import os
import glob
from display_bmo_bin import display_bin

ROOT = os.path.dirname(__file__)
REG_DIR = os.path.join(ROOT, '..', 'data', 'regions')
OUT_DIR = os.path.join(ROOT, 'simulated_faces')
os.makedirs(OUT_DIR, exist_ok=True)

for path in sorted(glob.glob(os.path.join(REG_DIR, '*.bin'))):
    print('Rendering', path)
    display_bin(path, OUT_DIR)
