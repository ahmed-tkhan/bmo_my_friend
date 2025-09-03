#!/usr/bin/env python3
"""
Generate test images for superpixel dithering evaluation.
Creates various patterns to test gray level granularity and superpixel effectiveness.
"""

import argparse
import os
import sys

W, H = 200, 200
BYTES = (W * H) // 4

def pack_4px(byte_val, lv, idx_in_byte):
    """Pack a 2-bit level into a byte at the specified position."""
    shift = (3 - idx_in_byte) * 2
    byte_val &= ~(0x3 << shift)
    byte_val |= (lv & 0x3) << shift
    return byte_val

def set_2bpp_level(buf, x, y, level):
    """Set a pixel level in the 2-bpp buffer."""
    if x < 0 or x >= W or y < 0 or y >= H:
        return
    level &= 0x3
    pix_index = y * W + x
    byte_index = pix_index >> 2
    idx_in_byte = pix_index & 3
    buf[byte_index] = pack_4px(buf[byte_index], level, idx_in_byte)

def create_gradient_image(output_path, direction='horizontal'):
    """Create a smooth gradient for testing gray level transitions."""
    buf = bytearray(BYTES)
    
    print(f"Creating {direction} gradient...")
    
    for y in range(H):
        for x in range(W):
            if direction == 'horizontal':
                level = (x * 3) // (W - 1)  # 0 to 3 left to right
            elif direction == 'vertical':
                level = (y * 3) // (H - 1)   # 0 to 3 top to bottom
            elif direction == 'diagonal':
                level = ((x + y) * 3) // (W + H - 2)  # diagonal gradient
            elif direction == 'radial':
                cx, cy = W//2, H//2
                dist = ((x - cx)**2 + (y - cy)**2)**0.5
                max_dist = ((cx**2 + cy**2)**0.5)
                level = int((dist * 3) / max_dist)
                if level > 3: level = 3
            
            set_2bpp_level(buf, x, y, level)
    
    with open(output_path, 'wb') as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {output_path}")

def create_calibration_pattern(output_path):
    """Create a calibration pattern with all 4 levels in strips."""
    buf = bytearray(BYTES)
    
    print("Creating calibration pattern...")
    
    # Horizontal strips of each level
    strip_height = H // 4
    
    for level in range(4):
        y_start = level * strip_height
        y_end = (level + 1) * strip_height
        
        for y in range(y_start, min(y_end, H)):
            for x in range(W):
                set_2bpp_level(buf, x, y, level)
        
        print(f"  Level {level}: rows {y_start}-{y_end-1}")
    
    with open(output_path, 'wb') as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {output_path}")

def create_step_pattern(output_path, steps=16):
    """Create a step pattern for testing perceived levels."""
    buf = bytearray(BYTES)
    
    print(f"Creating {steps}-step pattern...")
    
    step_width = W // steps
    
    for step in range(steps):
        # Map step to 2-bpp level (0-3)
        level = (step * 3) // (steps - 1)
        if level > 3: level = 3
        
        x_start = step * step_width
        x_end = (step + 1) * step_width
        
        for y in range(H):
            for x in range(x_start, min(x_end, W)):
                set_2bpp_level(buf, x, y, level)
        
        print(f"  Step {step:2d}: level {level}, cols {x_start:3d}-{x_end-1:3d}")
    
    with open(output_path, 'wb') as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {output_path}")

def create_checkerboard(output_path, square_size=8):
    """Create a checkerboard pattern for testing dithering effects."""
    buf = bytearray(BYTES)
    
    print(f"Creating checkerboard pattern (square size: {square_size})...")
    
    for y in range(H):
        for x in range(W):
            square_x = x // square_size
            square_y = y // square_size
            
            # Alternate between level 0 and 3 (white and black equivalent)
            level = 3 if (square_x + square_y) % 2 else 0
            set_2bpp_level(buf, x, y, level)
    
    with open(output_path, 'wb') as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {output_path}")

def create_dot_pattern(output_path):
    """Create a dot pattern for testing fine detail rendering."""
    buf = bytearray(BYTES)
    
    print("Creating dot pattern...")
    
    # Background level 1 (light gray)
    for y in range(H):
        for x in range(W):
            set_2bpp_level(buf, x, y, 1)
    
    # Add dots of different levels
    dot_spacing = 20
    
    for y in range(dot_spacing//2, H, dot_spacing):
        for x in range(dot_spacing//2, W, dot_spacing):
            # Determine dot level based on position
            level = ((x + y) // dot_spacing) % 4
            
            # Draw 3x3 dot
            for dy in range(-1, 2):
                for dx in range(-1, 2):
                    if 0 <= x+dx < W and 0 <= y+dy < H:
                        set_2bpp_level(buf, x+dx, y+dy, level)
    
    with open(output_path, 'wb') as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {output_path}")

def create_text_test(output_path):
    """Create a simple text-like pattern for readability testing."""
    buf = bytearray(BYTES)
    
    print("Creating text test pattern...")
    
    # Background level 0 (white)
    for y in range(H):
        for x in range(W):
            set_2bpp_level(buf, x, y, 0)
    
    # Create simple "TEXT" pattern using different gray levels
    patterns = [
        # Letter-like patterns at different gray levels
        (30, 30, 40, 60, 3),   # Dark block
        (60, 30, 40, 60, 2),   # Medium block
        (90, 30, 40, 60, 1),   # Light block
        (120, 30, 40, 60, 3),  # Dark block
        
        # Horizontal lines for readability test
        (10, 100, 180, 5, 3),   # Dark line
        (10, 110, 180, 5, 2),   # Medium line
        (10, 120, 180, 5, 1),   # Light line
        
        # Vertical lines
        (50, 140, 5, 40, 3),    # Dark vertical
        (70, 140, 5, 40, 2),    # Medium vertical
        (90, 140, 5, 40, 1),    # Light vertical
    ]
    
    for x_start, y_start, width, height, level in patterns:
        for y in range(y_start, min(y_start + height, H)):
            for x in range(x_start, min(x_start + width, W)):
                set_2bpp_level(buf, x, y, level)
    
    with open(output_path, 'wb') as f:
        f.write(buf)
    print(f"Wrote {len(buf)} bytes to {output_path}")

def main():
    parser = argparse.ArgumentParser(description="Generate test images for superpixel dithering evaluation")
    parser.add_argument("--output-dir", default=".", help="Output directory for test images")
    parser.add_argument("--all", action="store_true", help="Generate all test patterns")
    parser.add_argument("--gradient", action="store_true", help="Generate gradient test")
    parser.add_argument("--calibration", action="store_true", help="Generate calibration pattern")
    parser.add_argument("--steps", action="store_true", help="Generate step pattern")
    parser.add_argument("--checkerboard", action="store_true", help="Generate checkerboard")
    parser.add_argument("--dots", action="store_true", help="Generate dot pattern")
    parser.add_argument("--text", action="store_true", help="Generate text test")
    
    args = parser.parse_args()
    
    if not os.path.exists(args.output_dir):
        os.makedirs(args.output_dir)
        print(f"Created output directory: {args.output_dir}")
    
    if args.all or args.gradient:
        create_gradient_image(os.path.join(args.output_dir, "gradient_horizontal.bin"), "horizontal")
        create_gradient_image(os.path.join(args.output_dir, "gradient_vertical.bin"), "vertical")
        create_gradient_image(os.path.join(args.output_dir, "gradient_diagonal.bin"), "diagonal")
        create_gradient_image(os.path.join(args.output_dir, "gradient_radial.bin"), "radial")
    
    if args.all or args.calibration:
        create_calibration_pattern(os.path.join(args.output_dir, "calibration_strips.bin"))
    
    if args.all or args.steps:
        create_step_pattern(os.path.join(args.output_dir, "steps_16.bin"), 16)
        create_step_pattern(os.path.join(args.output_dir, "steps_8.bin"), 8)
    
    if args.all or args.checkerboard:
        create_checkerboard(os.path.join(args.output_dir, "checkerboard_8x8.bin"), 8)
        create_checkerboard(os.path.join(args.output_dir, "checkerboard_4x4.bin"), 4)
    
    if args.all or args.dots:
        create_dot_pattern(os.path.join(args.output_dir, "dot_pattern.bin"))
    
    if args.all or args.text:
        create_text_test(os.path.join(args.output_dir, "text_test.bin"))
    
    if not any([args.all, args.gradient, args.calibration, args.steps, args.checkerboard, args.dots, args.text]):
        print("No patterns specified. Use --all or specify individual patterns.")
        print("Available patterns: --gradient, --calibration, --steps, --checkerboard, --dots, --text")
        return 1
    
    print(f"\nTest images generated in: {args.output_dir}")
    print("Upload these to your ESP32's SPIFFS using:")
    print("  pio run --target uploadfs")
    print("\nOr copy to your data/ directory and upload filesystem.")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
