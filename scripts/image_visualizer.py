#!/usr/bin/env python3
"""
BMO Image Visualizer
Displays binary files and C arrays as images using matplotlib
Supports e-paper format analysis and BMO expression visualization
"""

import os
import sys
import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from pathlib import Path
import re

class BMOImageVisualizer:
    def __init__(self):
        self.project_root = Path(__file__).parent.parent
        self.data_dir = self.project_root / "data"
        
    def read_binary_file(self, filepath):
        """Read binary file and return bytes"""
        try:
            with open(filepath, 'rb') as f:
                return f.read()
        except Exception as e:
            print(f"Error reading file {filepath}: {e}")
            return None
    
    def parse_c_array(self, filepath):
        """Parse C array from header file and return bytes"""
        try:
            with open(filepath, 'r') as f:
                content = f.read()
            
            # Find array declarations
            pattern = r'const\s+unsigned\s+char\s+(\w+)\[\d*\]\s*(?:PROGMEM\s*)?=\s*\{([^}]+)\}'
            matches = re.findall(pattern, content, re.DOTALL)
            
            if not matches:
                print(f"No C arrays found in {filepath}")
                return None
                
            arrays = {}
            for array_name, array_data in matches:
                # Extract hex values
                hex_values = re.findall(r'0x([0-9A-Fa-f]{2})', array_data)
                byte_data = bytes(int(hex_val, 16) for hex_val in hex_values)
                arrays[array_name] = byte_data
                
            return arrays
        except Exception as e:
            print(f"Error parsing C array from {filepath}: {e}")
            return None
    
    def bytes_to_image(self, data, width, height, bit_depth=1):
        """Convert bytes to numpy image array"""
        if bit_depth == 1:
            # 1-bit: 8 pixels per byte
            expected_size = (width * height) // 8
            if len(data) != expected_size:
                raise ValueError(f"Data size {len(data)} doesn't match {width}x{height} 1-bit ({expected_size} bytes)")
            
            # Unpack bits
            image = np.zeros((height, width), dtype=np.uint8)
            for byte_idx, byte_val in enumerate(data):
                for bit_idx in range(8):
                    pixel_idx = byte_idx * 8 + bit_idx
                    if pixel_idx >= width * height:
                        break
                        
                    y = pixel_idx // width
                    x = pixel_idx % width
                    
                    # Extract bit (MSB first)
                    bit_val = (byte_val >> (7 - bit_idx)) & 1
                    image[y, x] = 255 if bit_val else 0  # White for 1, black for 0
                    
        elif bit_depth == 2:
            # 2-bit: 4 pixels per byte (4-grayscale)
            expected_size = (width * height) // 4
            if len(data) != expected_size:
                raise ValueError(f"Data size {len(data)} doesn't match {width}x{height} 2-bit ({expected_size} bytes)")
            
            image = np.zeros((height, width), dtype=np.uint8)
            for byte_idx, byte_val in enumerate(data):
                for pixel_in_byte in range(4):
                    pixel_idx = byte_idx * 4 + pixel_in_byte
                    if pixel_idx >= width * height:
                        break
                        
                    y = pixel_idx // width
                    x = pixel_idx % width
                    
                    # Extract 2-bit value (MSB first)
                    shift = 6 - (pixel_in_byte * 2)
                    pixel_val = (byte_val >> shift) & 0x03
                    
                    # Convert to grayscale (0=black, 1=dark gray, 2=light gray, 3=white)
                    gray_val = pixel_val * 85  # 255/3 ≈ 85
                    image[y, x] = gray_val
        else:
            raise ValueError(f"Unsupported bit depth: {bit_depth}")
            
        return image
    
    def auto_detect_format(self, data_size):
        """Auto-detect image format from data size"""
        formats = []
        
        # Try 1-bit formats
        for width in [200, 128, 104, 96, 64, 32]:
            for height in [200, 128, 104, 96, 64, 32]:
                if (width * height) // 8 == data_size:
                    formats.append((width, height, 1))
        
        # Try 2-bit formats  
        for width in [200, 128, 104, 96, 64, 32]:
            for height in [200, 128, 104, 96, 64, 32]:
                if (width * height) // 4 == data_size:
                    formats.append((width, height, 2))
        
        return formats
    
    def visualize_binary(self, filepath, width=None, height=None, bit_depth=None):
        """Visualize binary file as image"""
        data = self.read_binary_file(filepath)
        if data is None:
            return
            
        filename = Path(filepath).name
        
        # Auto-detect format if not specified
        if width is None or height is None or bit_depth is None:
            formats = self.auto_detect_format(len(data))
            if not formats:
                print(f"Cannot auto-detect format for {len(data)} bytes")
                return
            
            print(f"Possible formats for {filename}:")
            for i, (w, h, bd) in enumerate(formats):
                print(f"  {i+1}: {w}x{h} {bd}-bit")
            
            if len(formats) == 1:
                width, height, bit_depth = formats[0]
                print(f"Using: {width}x{height} {bit_depth}-bit")
            else:
                # Default to most likely format (200x200 1-bit for BMO)
                width, height, bit_depth = formats[0]
                print(f"Using first format: {width}x{height} {bit_depth}-bit")
        
        try:
            image = self.bytes_to_image(data, width, height, bit_depth)
            
            # Create figure
            fig, ax = plt.subplots(1, 1, figsize=(8, 8))
            
            # Display image
            if bit_depth == 1:
                ax.imshow(image, cmap='gray', vmin=0, vmax=255)
            else:
                ax.imshow(image, cmap='gray', vmin=0, vmax=255)
            
            ax.set_title(f"{filename}\n{width}x{height} {bit_depth}-bit ({len(data)} bytes)")
            ax.set_xlabel("X (pixels)")
            ax.set_ylabel("Y (pixels)")
            
            # Add grid for small images
            if width <= 64 and height <= 64:
                ax.set_xticks(np.arange(-0.5, width, 1), minor=True)
                ax.set_yticks(np.arange(-0.5, height, 1), minor=True)
                ax.grid(which="minor", color="red", linestyle='-', linewidth=0.5, alpha=0.3)
            
            plt.tight_layout()
            plt.show()
            
        except ValueError as e:
            print(f"Error creating image: {e}")
    
    def visualize_c_array(self, filepath, array_name=None):
        """Visualize C array from header file"""
        arrays = self.parse_c_array(filepath)
        if not arrays:
            return
            
        if array_name and array_name in arrays:
            # Visualize specific array
            data = arrays[array_name]
            print(f"Visualizing array: {array_name} ({len(data)} bytes)")
            self.visualize_from_data(data, f"{array_name} from {Path(filepath).name}")
        else:
            # Show all arrays
            print(f"Found {len(arrays)} arrays in {Path(filepath).name}:")
            for name, data in arrays.items():
                print(f"  - {name}: {len(data)} bytes")
                
            # Visualize all arrays
            for name, data in arrays.items():
                self.visualize_from_data(data, f"{name} from {Path(filepath).name}")
    
    def visualize_from_data(self, data, title):
        """Visualize image from raw data"""
        formats = self.auto_detect_format(len(data))
        if not formats:
            print(f"Cannot detect format for {len(data)} bytes in {title}")
            return
            
        # Use first detected format
        width, height, bit_depth = formats[0]
        
        try:
            image = self.bytes_to_image(data, width, height, bit_depth)
            
            fig, ax = plt.subplots(1, 1, figsize=(6, 6))
            ax.imshow(image, cmap='gray', vmin=0, vmax=255)
            ax.set_title(f"{title}\n{width}x{height} {bit_depth}-bit")
            ax.set_xlabel("X")
            ax.set_ylabel("Y")
            plt.tight_layout()
            plt.show()
            
        except ValueError as e:
            print(f"Error visualizing {title}: {e}")
    
    def show_all_expressions(self):
        """Show all BMO expression binaries in a grid"""
        bin_files = sorted(list(self.data_dir.glob("*.bin")))
        if not bin_files:
            print("No .bin files found in data directory")
            return
            
        print(f"Found {len(bin_files)} expression files")
        
        # Calculate grid size
        cols = min(4, len(bin_files))
        rows = (len(bin_files) + cols - 1) // cols
        
        fig, axes = plt.subplots(rows, cols, figsize=(12, 3*rows))
        if rows == 1:
            axes = [axes] if cols == 1 else axes
        else:
            axes = axes.flatten()
        
        for i, bin_file in enumerate(bin_files):
            data = self.read_binary_file(bin_file)
            if data is None:
                continue
                
            # Auto-detect format
            formats = self.auto_detect_format(len(data))
            if not formats:
                print(f"Skipping {bin_file.name} - unknown format")
                continue
                
            width, height, bit_depth = formats[0]
            
            try:
                image = self.bytes_to_image(data, width, height, bit_depth)
                
                ax = axes[i] if len(bin_files) > 1 else axes
                ax.imshow(image, cmap='gray', vmin=0, vmax=255)
                ax.set_title(f"{bin_file.stem}\n{width}x{height}")
                ax.set_xticks([])
                ax.set_yticks([])
                
            except Exception as e:
                print(f"Error processing {bin_file.name}: {e}")
        
        # Hide unused subplots
        for i in range(len(bin_files), len(axes)):
            axes[i].set_visible(False)
        
        plt.suptitle("BMO Facial Expressions", fontsize=16)
        plt.tight_layout()
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='Visualize BMO binary images and C arrays')
    parser.add_argument('--binary', '-b', help='Visualize binary file')
    parser.add_argument('--header', '-h', help='Visualize C arrays from header file')
    parser.add_argument('--array', '-a', help='Specific array name from header file')
    parser.add_argument('--width', '-w', type=int, help='Image width (auto-detect if not specified)')
    parser.add_argument('--height', '-y', type=int, help='Image height (auto-detect if not specified)')
    parser.add_argument('--bits', type=int, choices=[1, 2], help='Bit depth (1 or 2)')
    parser.add_argument('--all', action='store_true', help='Show all expression files in grid')
    
    args = parser.parse_args()
    
    visualizer = BMOImageVisualizer()
    
    if args.all:
        visualizer.show_all_expressions()
    elif args.binary:
        visualizer.visualize_binary(args.binary, args.width, args.height, args.bits)
    elif args.header:
        visualizer.visualize_c_array(args.header, args.array)
    else:
        print("Usage examples:")
        print("  python image_visualizer.py --all                           # Show all expressions")
        print("  python image_visualizer.py -b data/smile.bin               # Visualize binary")
        print("  python image_visualizer.py -h include/Ap_29demo.h          # Show all arrays")
        print("  python image_visualizer.py -h include/Ap_29demo.h -a Num   # Show specific array")

if __name__ == "__main__":
    main()
