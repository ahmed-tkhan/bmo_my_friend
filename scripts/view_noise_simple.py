#!/usr/bin/env python3
"""
Simple display script for 2-bpp Perlin noise binary files.
No external dependencies - shows ASCII art and hex dump.
"""

import argparse
import os
import sys

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

def crc32(data):
    """Simple CRC32 implementation."""
    crc = 0xFFFFFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xEDB88320
            else:
                crc >>= 1
    return crc ^ 0xFFFFFFFF

def display_binary_stats(filepath):
    """Show file statistics and hex dump."""
    
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found!")
        return False
    
    # Read binary data
    with open(filepath, 'rb') as f:
        data = f.read()
    
    file_size = len(data)
    print(f"File: {filepath}")
    print(f"Size: {file_size} bytes (expected: {BYTES} bytes)")
    print(f"Status: {'OK' if file_size == BYTES else 'WARNING - Size mismatch'}")
    
    if file_size == 0:
        print("File is empty!")
        return False
    
    # Calculate statistics
    level_counts = [0, 0, 0, 0]  # count each grayscale level
    
    for byte_idx in range(min(file_size, BYTES)):
        byte_val = data[byte_idx]
        four_pixels = unpack_4px(byte_val)
        for level in four_pixels:
            level_counts[level] += 1
    
    total_pixels = sum(level_counts)
    print(f"\nPixel Statistics ({total_pixels} total pixels):")
    print(f"  Level 0 (white):     {level_counts[0]:6d} pixels ({level_counts[0]*100/total_pixels:5.1f}%)")
    print(f"  Level 1 (light gray): {level_counts[1]:6d} pixels ({level_counts[1]*100/total_pixels:5.1f}%)")
    print(f"  Level 2 (dark gray):  {level_counts[2]:6d} pixels ({level_counts[2]*100/total_pixels:5.1f}%)")
    print(f"  Level 3 (black):     {level_counts[3]:6d} pixels ({level_counts[3]*100/total_pixels:5.1f}%)")
    
    # CRC32 checksum
    crc = crc32(data[:min(file_size, BYTES)])
    print(f"\nCRC32: 0x{crc:08X}")
    
    # Hex dump of first 128 bytes
    print(f"\nHex dump (first {min(128, file_size)} bytes):")
    for i in range(0, min(128, file_size), 16):
        hex_str = ""
        ascii_str = ""
        for j in range(16):
            if i + j < file_size:
                byte = data[i + j]
                hex_str += f"{byte:02X} "
                ascii_str += chr(byte) if 32 <= byte <= 126 else "."
            else:
                hex_str += "   "
                ascii_str += " "
        print(f"{i:04X}: {hex_str} |{ascii_str}|")
    
    if file_size > 128:
        print(f"... ({file_size - 128} more bytes)")
    
    return True

def display_binary_as_ascii(filepath, scale=4, chars=" ░▒█"):
    """Display 2-bpp binary as ASCII art."""
    
    if not os.path.exists(filepath):
        print(f"Error: File '{filepath}' not found!")
        return False
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    if len(data) == 0:
        print("File is empty!")
        return False
    
    print(f"\nASCII Art Preview (every {scale}x{scale} pixels, {len(chars)} levels):")
    
    # Sample the image at lower resolution for ASCII display
    display_w = W // scale
    display_h = H // scale
    
    print("+" + "-" * display_w + "+")
    
    for display_y in range(display_h):
        line = "|"
        for display_x in range(display_w):
            # Sample the center pixel of each scale x scale block
            src_x = display_x * scale + scale // 2
            src_y = display_y * scale + scale // 2
            
            pixel_idx = src_y * W + src_x
            
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
        
        line += "|"
        print(line)
    
    print("+" + "-" * display_w + "+")
    print(f"Legend: '{chars[0]}'=white, '{chars[1]}'=light gray, '{chars[2]}'=dark gray, '{chars[3]}'=black")
    
    return True

def analyze_noise_pattern(filepath):
    """Analyze the noise pattern characteristics."""
    
    if not os.path.exists(filepath):
        return False
    
    with open(filepath, 'rb') as f:
        data = f.read()
    
    if len(data) < BYTES:
        return False
    
    print(f"\nNoise Pattern Analysis:")
    
    # Check for obvious patterns
    all_same = all(b == data[0] for b in data[:100])  # Check first 100 bytes
    if all_same:
        print("  WARNING: Pattern appears uniform (all same values)")
        return True
    
    # Look for repeating patterns
    pattern_found = False
    for pattern_len in [1, 2, 4, 8, 16]:
        if len(data) >= pattern_len * 4:  # Need at least 4 repetitions
            pattern = data[:pattern_len]
            repeats = 0
            for i in range(pattern_len, min(len(data), pattern_len * 10), pattern_len):
                if data[i:i+pattern_len] == pattern:
                    repeats += 1
                else:
                    break
            if repeats >= 3:
                print(f"  Found repeating {pattern_len}-byte pattern (repeats {repeats} times)")
                pattern_found = True
                break
    
    if not pattern_found:
        print("  Pattern appears random (good for Perlin noise)")
    
    # Check distribution
    byte_counts = {}
    for b in data[:1000]:  # Sample first 1000 bytes
        byte_counts[b] = byte_counts.get(b, 0) + 1
    
    unique_bytes = len(byte_counts)
    print(f"  Unique byte values in sample: {unique_bytes}/256 ({unique_bytes*100/256:.1f}%)")
    
    return True

def main():
    parser = argparse.ArgumentParser(description="Display 200x200 2-bpp Perlin noise binary file (no external deps)")
    parser.add_argument("input", help="Input binary file path")
    parser.add_argument("--ascii", action="store_true", help="Show ASCII art preview")
    parser.add_argument("--scale", type=int, default=4, help="ASCII art scale factor (default: 4)")
    parser.add_argument("--analyze", action="store_true", help="Analyze noise pattern")
    parser.add_argument("--chars", default=" ░▒█", help="Characters for ASCII art levels")
    
    args = parser.parse_args()
    
    # Check if file exists
    if not os.path.exists(args.input):
        print(f"Error: Input file '{args.input}' not found!")
        
        # Look for the default file in common locations
        search_paths = [
            os.path.join(os.path.dirname(__file__), "..", "data", "noise_200x200_2bpp.bin"),
            os.path.join(os.path.dirname(args.input), "noise_200x200_2bpp.bin"),
            "noise_200x200_2bpp.bin"
        ]
        
        for path in search_paths:
            if os.path.exists(path):
                print(f"Found file at: {path}")
                args.input = path
                break
        else:
            print("\nTo generate the noise file, run:")
            print("  python scripts/pearlian.py")
            return 1
    
    print("=" * 60)
    print("Perlin Noise Binary File Viewer")
    print("=" * 60)
    
    # Show basic stats
    success = display_binary_stats(args.input)
    if not success:
        return 1
    
    # Show ASCII art if requested
    if args.ascii:
        display_binary_as_ascii(args.input, args.scale, args.chars)
    
    # Analyze pattern if requested
    if args.analyze:
        analyze_noise_pattern(args.input)
    
    print("\n" + "=" * 60)
    print("Tip: Use --ascii to see visual preview")
    print("Tip: Use --analyze to check noise quality")
    print("=" * 60)
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
