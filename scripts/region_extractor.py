#!/usr/bin/env python3
"""
BMO Region Extractor
Extract specific regions (eyes, mouth) from full BMO face images
Create separate region files for 2-region animation system
"""

import os
import sys
import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from pathlib import Path
import json

class BMORegionExtractor:
    def __init__(self):
        self.project_root = Path(__file__).parent.parent
        self.data_dir = self.project_root / "data"
        self.regions_config = self.data_dir / "bmo_face_regions.json"
        
        # Default regions for 200x200 BMO face (can be overridden by JSON)
        self.default_regions = {
            "eyes": {
                "x": 50,
                "y": 60, 
                "width": 100,
                "height": 40,
                "description": "Eyes region - includes both eyes"
            },
            "mouth": {
                "x": 70,
                "y": 120,
                "width": 60, 
                "height": 30,
                "description": "Mouth region - includes mouth area"
            }
        }
        
    def load_regions_config(self):
        """Load region definitions from JSON file"""
        if self.regions_config.exists():
            try:
                with open(self.regions_config, 'r') as f:
                    config = json.load(f)
                    return config.get('regions', self.default_regions)
            except Exception as e:
                print(f"Error loading regions config: {e}")
                
        return self.default_regions
    
    def read_binary_file(self, filepath):
        """Read binary file and return bytes"""
        try:
            with open(filepath, 'rb') as f:
                return f.read()
        except Exception as e:
            print(f"Error reading file {filepath}: {e}")
            return None
    
    def bytes_to_image(self, data, width, height, bit_depth=1):
        """Convert bytes to numpy image array"""
        if bit_depth == 1:
            expected_size = (width * height) // 8
            if len(data) != expected_size:
                raise ValueError(f"Data size {len(data)} doesn't match {width}x{height} 1-bit")
            
            image = np.zeros((height, width), dtype=np.uint8)
            for byte_idx, byte_val in enumerate(data):
                for bit_idx in range(8):
                    pixel_idx = byte_idx * 8 + bit_idx
                    if pixel_idx >= width * height:
                        break
                        
                    y = pixel_idx // width
                    x = pixel_idx % width
                    bit_val = (byte_val >> (7 - bit_idx)) & 1
                    image[y, x] = 255 if bit_val else 0
                    
        return image
    
    def image_to_bytes(self, image, bit_depth=1):
        """Convert numpy image array back to bytes"""
        height, width = image.shape
        
        if bit_depth == 1:
            # Pack 8 pixels per byte
            total_bytes = (width * height + 7) // 8
            data = bytearray(total_bytes)
            
            for y in range(height):
                for x in range(width):
                    pixel_idx = y * width + x
                    byte_idx = pixel_idx // 8
                    bit_idx = pixel_idx % 8
                    
                    if byte_idx >= len(data):
                        break
                        
                    # Convert pixel to bit (>127 = white = 1, <=127 = black = 0)
                    bit_val = 1 if image[y, x] > 127 else 0
                    
                    # Set bit in byte (MSB first)
                    if bit_val:
                        data[byte_idx] |= (1 << (7 - bit_idx))
            
            return bytes(data)
        
        raise ValueError(f"Unsupported bit depth: {bit_depth}")
    
    def extract_region(self, image, region_config):
        """Extract a region from the full image"""
        x = region_config['x']
        y = region_config['y'] 
        width = region_config['width']
        height = region_config['height']
        
        # Validate bounds
        img_height, img_width = image.shape
        if x + width > img_width or y + height > img_height:
            raise ValueError(f"Region {x},{y} {width}x{height} exceeds image bounds {img_width}x{img_height}")
        
        # Extract region
        region = image[y:y+height, x:x+width]
        return region
    
    def process_expression(self, input_path, output_dir=None):
        """Extract regions from a BMO expression file"""
        input_path = Path(input_path)
        if output_dir is None:
            output_dir = self.data_dir / "regions"
        output_dir = Path(output_dir)
        output_dir.mkdir(exist_ok=True)
        
        # Read input file
        data = self.read_binary_file(input_path)
        if data is None:
            return False
            
        # Auto-detect format (assume 200x200 1-bit for BMO)
        width, height, bit_depth = 200, 200, 1
        expected_size = (width * height) // 8
        
        if len(data) != expected_size:
            print(f"Warning: File size {len(data)} doesn't match expected {expected_size} for 200x200 1-bit")
            # Try other common formats
            formats = [(128, 128, 1), (96, 96, 1), (64, 64, 1)]
            for w, h, bd in formats:
                if len(data) == (w * h) // 8:
                    width, height, bit_depth = w, h, bd
                    print(f"Using format: {width}x{height} {bit_depth}-bit")
                    break
            else:
                print(f"Cannot determine format for {len(data)} bytes")
                return False
        
        try:
            # Convert to image
            image = self.bytes_to_image(data, width, height, bit_depth)
            
            # Load region configurations
            regions = self.load_regions_config()
            
            # Extract each region
            expression_name = input_path.stem
            results = {}
            
            for region_name, region_config in regions.items():
                try:
                    region_image = self.extract_region(image, region_config)
                    region_data = self.image_to_bytes(region_image, bit_depth)
                    
                    # Save region file
                    output_file = output_dir / f"{expression_name}_{region_name}.bin"
                    with open(output_file, 'wb') as f:
                        f.write(region_data)
                    
                    results[region_name] = {
                        'file': output_file,
                        'size': len(region_data),
                        'dimensions': f"{region_config['width']}x{region_config['height']}"
                    }
                    
                    print(f"Extracted {region_name}: {results[region_name]['dimensions']} -> {output_file.name}")
                    
                except Exception as e:
                    print(f"Error extracting {region_name}: {e}")
            
            return results
            
        except Exception as e:
            print(f"Error processing {input_path}: {e}")
            return False
    
    def visualize_regions(self, input_path):
        """Visualize the regions overlay on the original image"""
        data = self.read_binary_file(input_path)
        if data is None:
            return
            
        # Assume 200x200 1-bit
        width, height, bit_depth = 200, 200, 1
        try:
            image = self.bytes_to_image(data, width, height, bit_depth)
            regions = self.load_regions_config()
            
            fig, ax = plt.subplots(1, 1, figsize=(8, 8))
            ax.imshow(image, cmap='gray', vmin=0, vmax=255)
            
            # Draw region rectangles
            colors = ['red', 'blue', 'green', 'orange', 'purple']
            for i, (region_name, region_config) in enumerate(regions.items()):
                rect = patches.Rectangle(
                    (region_config['x'], region_config['y']),
                    region_config['width'], 
                    region_config['height'],
                    linewidth=2, 
                    edgecolor=colors[i % len(colors)], 
                    facecolor='none',
                    label=region_name
                )
                ax.add_patch(rect)
                
                # Add label
                ax.text(
                    region_config['x'] + 5, 
                    region_config['y'] + 15,
                    region_name,
                    color=colors[i % len(colors)],
                    fontweight='bold',
                    fontsize=10
                )
            
            ax.set_title(f"BMO Regions: {Path(input_path).name}")
            ax.set_xlabel("X (pixels)")
            ax.set_ylabel("Y (pixels)")
            ax.legend()
            plt.tight_layout()
            plt.show()
            
        except Exception as e:
            print(f"Error visualizing regions: {e}")
    
    def process_all_expressions(self):
        """Process all BMO expression files in data directory"""
        bin_files = list(self.data_dir.glob("*.bin"))
        if not bin_files:
            print("No .bin files found in data directory")
            return
            
        print(f"Processing {len(bin_files)} expression files...")
        
        output_dir = self.data_dir / "regions"
        success_count = 0
        
        for bin_file in sorted(bin_files):
            print(f"\nProcessing: {bin_file.name}")
            result = self.process_expression(bin_file, output_dir)
            if result:
                success_count += 1
        
        print(f"\nCompleted: {success_count}/{len(bin_files)} files processed successfully")
        print(f"Region files saved to: {output_dir}")

def main():
    parser = argparse.ArgumentParser(description='Extract BMO face regions for animation')
    parser.add_argument('--file', '-f', help='Process specific expression file')
    parser.add_argument('--all', '-a', action='store_true', help='Process all expression files')
    parser.add_argument('--visualize', '-v', help='Visualize regions on image')
    parser.add_argument('--output', '-o', help='Output directory for region files')
    
    args = parser.parse_args()
    
    extractor = BMORegionExtractor()
    
    if args.visualize:
        extractor.visualize_regions(args.visualize)
    elif args.file:
        result = extractor.process_expression(args.file, args.output)
        if result:
            print(f"\nExtracted regions from {args.file}:")
            for region, info in result.items():
                print(f"  {region}: {info['dimensions']} ({info['size']} bytes)")
    elif args.all:
        extractor.process_all_expressions()
    else:
        print("Usage examples:")
        print("  python region_extractor.py --all                        # Process all expressions")
        print("  python region_extractor.py -f data/smile.bin            # Process specific file")
        print("  python region_extractor.py -v data/smile.bin            # Visualize regions")

if __name__ == "__main__":
    main()
