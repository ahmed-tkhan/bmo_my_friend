#!/usr/bin/env python3
"""
BMO Tools Master Script
Central command interface for all BMO image processing tools
"""

import sys
import argparse
from pathlib import Path

# Add scripts directory to path
sys.path.append(str(Path(__file__).parent))

from binary_converter import BinaryToArrayConverter
from image_visualizer import BMOImageVisualizer  
from region_extractor import BMORegionExtractor

def main():
    parser = argparse.ArgumentParser(
        description='BMO Image Processing Tools',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # View all BMO expressions
  python bmo_tools.py view --all
  
  # Convert all binaries to C arrays
  python bmo_tools.py convert --all
  
  # Extract regions from all expressions
  python bmo_tools.py extract --all
  
  # Analyze a specific file
  python bmo_tools.py analyze data/smile.bin
  
  # Complete workflow: view -> extract -> convert
  python bmo_tools.py workflow
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # VIEW command
    view_parser = subparsers.add_parser('view', help='Visualize images')
    view_parser.add_argument('--all', action='store_true', help='Show all expressions in grid')
    view_parser.add_argument('--file', '-f', help='View specific binary file') 
    view_parser.add_argument('--header', help='View C arrays from header file')
    view_parser.add_argument('--regions', '-r', help='Show regions overlay on image')
    
    # CONVERT command
    convert_parser = subparsers.add_parser('convert', help='Convert binaries to C arrays')
    convert_parser.add_argument('--all', action='store_true', help='Convert all .bin files')
    convert_parser.add_argument('--file', '-f', help='Convert specific file')
    convert_parser.add_argument('--name', '-n', help='Custom array name')
    
    # EXTRACT command  
    extract_parser = subparsers.add_parser('extract', help='Extract face regions')
    extract_parser.add_argument('--all', action='store_true', help='Extract from all expressions')
    extract_parser.add_argument('--file', '-f', help='Extract from specific file')
    extract_parser.add_argument('--output', '-o', help='Output directory')
    
    # ANALYZE command
    analyze_parser = subparsers.add_parser('analyze', help='Analyze binary file format')
    analyze_parser.add_argument('file', help='Binary file to analyze')
    
    # WORKFLOW command
    workflow_parser = subparsers.add_parser('workflow', help='Complete processing workflow')
    workflow_parser.add_argument('--skip-view', action='store_true', help='Skip visualization step')
    
    # STATUS command
    status_parser = subparsers.add_parser('status', help='Show project status')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    # Initialize tools
    converter = BinaryToArrayConverter()
    visualizer = BMOImageVisualizer()
    extractor = BMORegionExtractor()
    
    try:
        if args.command == 'view':
            if args.all:
                visualizer.show_all_expressions()
            elif args.file:
                visualizer.visualize_binary(args.file)
            elif args.header:
                visualizer.visualize_c_array(args.header)
            elif args.regions:
                extractor.visualize_regions(args.regions)
            else:
                print("Specify --all, --file, --header, or --regions")
                
        elif args.command == 'convert':
            if args.all:
                converter.convert_all_bins_in_data()
            elif args.file:
                result = converter.convert_file(args.file, args.name)
                if result:
                    print("\nGenerated C Array:")
                    print("=" * 50)
                    print(result)
            else:
                print("Specify --all or --file")
                
        elif args.command == 'extract':
            if args.all:
                extractor.process_all_expressions()
            elif args.file:
                result = extractor.process_expression(args.file, args.output)
                if result:
                    print(f"\nExtracted regions from {args.file}:")
                    for region, info in result.items():
                        print(f"  {region}: {info['dimensions']} ({info['size']} bytes)")
            else:
                print("Specify --all or --file")
                
        elif args.command == 'analyze':
            converter.analyze_binary_file(args.file)
            
        elif args.command == 'workflow':
            print("=== BMO Complete Processing Workflow ===\n")
            
            # Step 1: Show current expressions (optional)
            if not args.skip_view:
                print("Step 1: Viewing all BMO expressions...")
                visualizer.show_all_expressions()
                input("Press Enter to continue to region extraction...")
            
            # Step 2: Extract regions
            print("Step 2: Extracting face regions...")
            extractor.process_all_expressions()
            
            # Step 3: Convert to C arrays
            print("\nStep 3: Converting to C header arrays...")
            converter.convert_all_bins_in_data()
            
            print("\n=== Workflow Complete! ===")
            print("Next steps:")
            print("1. Check include/bmo_expressions.h for C arrays")
            print("2. Check data/regions/ for extracted region files")
            print("3. Test with ESP32 firmware")
            
        elif args.command == 'status':
            show_project_status()
            
    except KeyboardInterrupt:
        print("\nOperation cancelled by user")
    except Exception as e:
        print(f"Error: {e}")

def show_project_status():
    """Show current project status and files"""
    project_root = Path(__file__).parent.parent
    data_dir = project_root / "data"
    include_dir = project_root / "include"
    regions_dir = data_dir / "regions"
    
    print("=== BMO Project Status ===\n")
    
    # Count files
    bin_files = list(data_dir.glob("*.bin")) if data_dir.exists() else []
    region_files = list(regions_dir.glob("*.bin")) if regions_dir.exists() else []
    header_files = list(include_dir.glob("*.h")) if include_dir.exists() else []
    
    print(f"Expression binaries: {len(bin_files)} files")
    for f in sorted(bin_files):
        print(f"  - {f.name}")
    
    print(f"\nExtracted regions: {len(region_files)} files")
    if region_files:
        # Group by expression
        expressions = {}
        for f in region_files:
            expr_name = f.stem.split('_')[0]
            if expr_name not in expressions:
                expressions[expr_name] = []
            expressions[expr_name].append(f.name)
        
        for expr, files in sorted(expressions.items()):
            print(f"  {expr}: {', '.join(files)}")
    
    print(f"\nHeader files: {len(header_files)} files")
    for f in sorted(header_files):
        print(f"  - {f.name}")
    
    # Check for generated files
    bmo_expressions = include_dir / "bmo_expressions.h"
    if bmo_expressions.exists():
        print(f"\n✓ Generated C arrays: {bmo_expressions}")
    else:
        print(f"\n✗ Missing: {bmo_expressions} (run convert --all)")
    
    regions_config = data_dir / "bmo_face_regions.json"
    if regions_config.exists():
        print(f"✓ Regions config: {regions_config}")
    else:
        print(f"✗ Missing: {regions_config}")
    
    print(f"\nDirectories:")
    print(f"  Data: {data_dir}")
    print(f"  Include: {include_dir}")
    print(f"  Regions: {regions_dir}")

if __name__ == "__main__":
    main()
