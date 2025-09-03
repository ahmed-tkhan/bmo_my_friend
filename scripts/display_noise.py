#!/usr/bin/env python3
"""
Display script for 2-bpp Perlin noise binary files.
Shows the binary data as a grayscale image with 4 levels (0=white, 1=light gray, 2=dark gray, 3=black).
"""

import argparse
import struct
import sys
from PIL import Image
import os

W, H = 200, 200  # image dimensions
BYTES = (W * H) // 4  # 10,000 bytes for 2-bpp

def unpack_4px(byte_val):
    """Extract 4 pixels (2-bit each) from a single byte."""
    # Big-endian: bits 7-6, 5-4, 3-2, 1-0
    px0 = (byte_val >> 6) & 0x3
    px1 = (byte_val >> 4) & 0x3
    px2 = (byte_val >> 2) & 0x3
    px3 = (byte_val >> 0) & 0x3
    return [px0, px1, px2, px3]

def level_to_gray(level):
    """Convert 2-bit level to 8-bit grayscale value."""
    # 0=white, 1=light gray, 2=dark gray, 3=black
    gray_map = [255, 170, 85, 0]  # 0xFF, 0xAA, 0x55, 0x00
    return gray_map[level & 0x3]

def display_binary_as_image(filepath, save_png=None, show_stats=True):
    """Load and display 2-bpp binary file as grayscale image."""
    
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found!")
        return False
    
    # Read binary data
    with open(filepath, 'rb') as f:
        data = f.read()
    
    if len(data) != BYTES:
        print(f"Warning: Expected {BYTES} bytes, got {len(data)} bytes")
        if len(data) < BYTES:
            print("File too small!")
            return False
    
    print(f"Loaded {len(data)} bytes from '{filepath}'")
    
    # Create image array
    pixels = []
    level_counts = [0, 0, 0, 0]  # count each grayscale level
    
    for byte_idx in range(min(len(data), BYTES)):
        byte_val = data[byte_idx]
        four_pixels = unpack_4px(byte_val)
        
        for level in four_pixels:
            pixels.append(level_to_gray(level))
            level_counts[level] += 1
    
    # Pad if necessary
    while len(pixels) < W * H:
        pixels.append(0)  # black padding
        level_counts[3] += 1
    
    # Create PIL image
    img = Image.new('L', (W, H))  # 'L' mode = grayscale
    img.putdata(pixels)
    
    # Display statistics
    if show_stats:
        print(f"\nImage Statistics ({W}x{H}):")
        print(f"  Level 0 (white):     {level_counts[0]:5d} pixels ({level_counts[0]*100/(W*H):5.1f}%)")
        print(f"  Level 1 (light gray): {level_counts[1]:5d} pixels ({level_counts[1]*100/(W*H):5.1f}%)")
        print(f"  Level 2 (dark gray):  {level_counts[2]:5d} pixels ({level_counts[2]*100/(W*H):5.1f}%)")
        print(f"  Level 3 (black):     {level_counts[3]:5d} pixels ({level_counts[3]*100/(W*H):5.1f}%)")
        
        # Calculate CRC32 for verification
        import zlib
        crc = zlib.crc32(data[:BYTES]) & 0xffffffff
        print(f"\nFile CRC32: 0x{crc:08X}")
        
        # Show first few bytes as hex
        hex_preview = ' '.join(f'{b:02X}' for b in data[:16])
        print(f"First 16 bytes: {hex_preview}")
    
    # Save as PNG if requested
    if save_png:
        img.save(save_png)
        print(f"Saved PNG preview to '{save_png}'")
    
    # Show the image
    try:
        img.show()
        print("Image displayed (close window to continue)")
    except Exception as e:
        print(f"Could not display image: {e}")
        print("Try installing Pillow with: pip install Pillow")
    
    return True

def display_binary_as_ascii(filepath, chars=" ░▒█"):
    """Display 2-bpp binary as ASCII art."""
    
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found!")
        return False
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    print(f"\nASCII Preview ({len(chars)} levels):")
    print("+" + "-" * W + "+")
    
    pixel_idx = 0
    for y in range(H):
        line = "|"
        for x in range(W):
            if pixel_idx < len(data) * 4:
                byte_idx = pixel_idx // 4
                bit_pos = (3 - (pixel_idx % 4)) * 2
                if byte_idx < len(data):
                    level = (data[byte_idx] >> bit_pos) & 0x3
                    line += chars[level]
                else:
                    line += chars[3]  # black for missing data
            else:
                line += chars[3]
            pixel_idx += 1
        line += "|"
        
        # Only print every 4th line for readability
        if y % 4 == 0:
            print(line)
    
    print("+" + "-" * W + "+")
    return True

def main():
    parser = argparse.ArgumentParser(description="Display 200x200 2-bpp Perlin noise binary file")
    parser.add_argument("input", help="Input binary file path")
    parser.add_argument("--png", help="Save PNG preview to this file")
    parser.add_argument("--ascii", action="store_true", help="Show ASCII art preview")
    parser.add_argument("--no-stats", action="store_true", help="Don't show statistics")
    parser.add_argument("--no-image", action="store_true", help="Don't show GUI image")
    
    args = parser.parse_args()
    
    # Check if file exists
    if not os.path.exists(args.input):
        print(f"Error: Input file '{args.input}' not found!")
        
        # Look for the default file
        default_path = os.path.join(os.path.dirname(args.input), "noise_200x200_2bpp.bin")
        if os.path.exists(default_path):
            print(f"Found default file: {default_path}")
            args.input = default_path
        else:
            return 1
    
    success = True
    
    # Display as image (unless disabled)
    if not args.no_image:
        success = display_binary_as_image(args.input, args.png, not args.no_stats)
    
    # Display as ASCII if requested
    if args.ascii:
        display_binary_as_ascii(args.input)
    
    return 0 if success else 1

if __name__ == "__main__":
    sys.exit(main())
