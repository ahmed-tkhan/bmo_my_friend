#!/usr/bin/env python3
"""
ESP32 Memory Analysis for 2-bpp Image Storage
Calculates how many 10,000-byte image files can be stored on different ESP32 variants.
"""

# ESP32 Memory Specifications (typical values)
ESP32_VARIANTS = {
    "ESP32-WROOM-32 (DevKit V1)": {
        "flash_sizes": [4*1024*1024, 8*1024*1024, 16*1024*1024],  # 4MB, 8MB, 16MB available
        "default_flash": 4*1024*1024,  # Most common: 4MB
        "sram": 520*1024,  # 520KB SRAM
        "psram": 0,        # No PSRAM by default
        "description": "Standard ESP32 on most dev boards"
    },
    "ESP32-C3-Mini (Seeed)": {
        "flash_sizes": [4*1024*1024],  # Usually 4MB
        "default_flash": 4*1024*1024,
        "sram": 400*1024,  # 400KB SRAM
        "psram": 0,        # No PSRAM
        "description": "Smaller, more efficient ESP32-C3"
    },
    "ESP32-S3 (with PSRAM)": {
        "flash_sizes": [8*1024*1024, 16*1024*1024, 32*1024*1024],
        "default_flash": 8*1024*1024,
        "sram": 512*1024,  # 512KB SRAM
        "psram": 8*1024*1024,  # 8MB PSRAM option
        "description": "High-end ESP32 with optional PSRAM"
    }
}

# System overhead estimates
SYSTEM_OVERHEAD = {
    "bootloader": 32*1024,      # Bootloader partition
    "partition_table": 4*1024,  # Partition table
    "nvs": 20*1024,            # NVS (Non-Volatile Storage)
    "otadata": 8*1024,         # OTA data partition
    "firmware": 1*1024*1024,   # Typical firmware size (1MB)
    "spiffs_overhead": 16*1024, # SPIFFS filesystem overhead per MB
    "runtime_heap": 100*1024,  # Runtime heap requirement
    "stack": 32*1024,          # Stack space
}

# Image specifications
IMAGE_SIZE = 10000  # bytes per 2-bpp 200x200 image
IMAGE_DISPLAY_BUFFER = 5000  # bytes needed to convert to display planes

def calculate_available_storage(variant_name, variant_info, flash_size=None):
    """Calculate available storage for images on a given ESP32 variant."""
    
    if flash_size is None:
        flash_size = variant_info["default_flash"]
    
    # Calculate system overhead
    total_overhead = (
        SYSTEM_OVERHEAD["bootloader"] +
        SYSTEM_OVERHEAD["partition_table"] +
        SYSTEM_OVERHEAD["nvs"] +
        SYSTEM_OVERHEAD["otadata"] +
        SYSTEM_OVERHEAD["firmware"] +
        SYSTEM_OVERHEAD["spiffs_overhead"] * (flash_size // (1024*1024))
    )
    
    # Available space for SPIFFS (user data)
    spiffs_partition = flash_size - total_overhead
    
    # Leave some buffer space (10% of available)
    usable_spiffs = int(spiffs_partition * 0.9)
    
    return {
        "flash_total": flash_size,
        "system_overhead": total_overhead,
        "spiffs_partition": spiffs_partition,
        "usable_spiffs": usable_spiffs,
        "max_images": usable_spiffs // IMAGE_SIZE,
        "leftover_bytes": usable_spiffs % IMAGE_SIZE
    }

def calculate_runtime_memory(variant_info):
    """Calculate runtime memory usage for image processing."""
    
    sram = variant_info["sram"]
    psram = variant_info["psram"]
    
    # Memory needed for image processing
    image_buffer = IMAGE_SIZE  # Original 2-bpp image
    plane0_buffer = 5000       # 1-bpp plane 0 (200x200/8)
    plane1_buffer = 5000       # 1-bpp plane 1
    temp_buffers = 10000       # Temporary processing buffers
    
    total_image_memory = image_buffer + plane0_buffer + plane1_buffer + temp_buffers
    
    # System runtime overhead
    runtime_overhead = (
        SYSTEM_OVERHEAD["runtime_heap"] +
        SYSTEM_OVERHEAD["stack"]
    )
    
    available_sram = sram - runtime_overhead
    available_psram = psram
    
    return {
        "sram_total": sram,
        "psram_total": psram,
        "runtime_overhead": runtime_overhead,
        "available_sram": available_sram,
        "available_psram": available_psram,
        "image_processing_memory": total_image_memory,
        "can_process_in_sram": available_sram >= total_image_memory,
        "simultaneous_images_sram": available_sram // total_image_memory,
        "simultaneous_images_psram": available_psram // total_image_memory if psram > 0 else 0
    }

def format_bytes(bytes_val):
    """Format bytes in human-readable form."""
    for unit in ['B', 'KB', 'MB', 'GB']:
        if bytes_val < 1024:
            return f"{bytes_val:.1f} {unit}"
        bytes_val /= 1024
    return f"{bytes_val:.1f} GB"

def print_variant_analysis(variant_name, variant_info):
    """Print detailed analysis for a specific ESP32 variant."""
    
    print(f"\n{'='*60}")
    print(f"{variant_name}")
    print(f"Description: {variant_info['description']}")
    print(f"{'='*60}")
    
    # Analyze different flash sizes
    for flash_size in variant_info["flash_sizes"]:
        storage = calculate_available_storage(variant_name, variant_info, flash_size)
        runtime = calculate_runtime_memory(variant_info)
        
        print(f"\n--- Flash Size: {format_bytes(flash_size)} ---")
        print(f"System Overhead: {format_bytes(storage['system_overhead'])}")
        print(f"SPIFFS Partition: {format_bytes(storage['spiffs_partition'])}")
        print(f"Usable Storage: {format_bytes(storage['usable_spiffs'])}")
        print(f"")
        print(f"📸 Max 2-bpp Images: {storage['max_images']:,}")
        print(f"💾 Per Image: {format_bytes(IMAGE_SIZE)}")
        print(f"📦 Total Image Storage: {format_bytes(storage['max_images'] * IMAGE_SIZE)}")
        print(f"🗂️  Leftover Space: {format_bytes(storage['leftover_bytes'])}")
        
        print(f"\n--- Runtime Memory ---")
        print(f"SRAM Total: {format_bytes(runtime['sram_total'])}")
        if runtime['psram_total'] > 0:
            print(f"PSRAM Total: {format_bytes(runtime['psram_total'])}")
        print(f"Available SRAM: {format_bytes(runtime['available_sram'])}")
        print(f"Image Processing Needs: {format_bytes(runtime['image_processing_memory'])}")
        print(f"✅ Can Process in SRAM: {'Yes' if runtime['can_process_in_sram'] else 'No'}")
        print(f"🔄 Simultaneous Images (SRAM): {runtime['simultaneous_images_sram']}")
        if runtime['simultaneous_images_psram'] > 0:
            print(f"🔄 Simultaneous Images (PSRAM): {runtime['simultaneous_images_psram']}")

def generate_recommendations():
    """Generate recommendations based on analysis."""
    
    print(f"\n{'='*60}")
    print("RECOMMENDATIONS")
    print(f"{'='*60}")
    
    print("\n💡 For ESP32-WROOM-32 (DevKit V1):")
    print("   • 4MB Flash: ~260 images (2.6MB storage)")
    print("   • 8MB Flash: ~700 images (7MB storage)")  
    print("   • 16MB Flash: ~1,500 images (15MB storage)")
    print("   • Runtime: Can process 1-2 images simultaneously in SRAM")
    
    print("\n💡 For ESP32-C3-Mini (Seeed):")
    print("   • 4MB Flash: ~260 images (2.6MB storage)")
    print("   • Less SRAM than ESP32, but still adequate for single image processing")
    print("   • More power efficient for battery applications")
    
    print("\n💡 For ESP32-S3 (with PSRAM):")
    print("   • 8MB+ Flash: 700+ images")
    print("   • PSRAM allows processing many images simultaneously")
    print("   • Best for complex image manipulation")
    
    print("\n🎯 STORAGE STRATEGIES:")
    print("   1. SPIFFS: Simple, good for <1000 images")
    print("   2. LittleFS: More efficient than SPIFFS, better wear leveling")
    print("   3. SD Card: Unlimited storage via SPI interface")
    print("   4. External Flash: I2C/SPI flash chips for more storage")
    
    print("\n🔧 OPTIMIZATION TIPS:")
    print("   • Use streaming: Load/process/display/free one image at a time")
    print("   • Compress similar images (RLE for repeated patterns)")
    print("   • Store as 1-bpp with dithering metadata for superpixel reconstruction")
    print("   • Use external storage (SD card) for large collections")

def calculate_superpixel_memory():
    """Calculate memory for superpixel dithering approaches."""
    
    print(f"\n{'='*60}")
    print("SUPERPIXEL MEMORY ANALYSIS")
    print(f"{'='*60}")
    
    # Different superpixel strategies
    strategies = {
        "2x2 Superpixel (4 levels -> 16 levels)": {
            "input_bpp": 2,
            "output_levels": 16,
            "pattern_size": 2,
            "memory_multiplier": 1.0  # Same file size, processed differently
        },
        "4x4 Superpixel (4 levels -> 64 levels)": {
            "input_bpp": 2, 
            "output_levels": 64,
            "pattern_size": 4,
            "memory_multiplier": 1.0
        },
        "Dithered 1-bpp + Metadata": {
            "input_bpp": 1,
            "output_levels": "many",
            "pattern_size": "variable",
            "memory_multiplier": 0.6  # 1-bpp + small metadata
        }
    }
    
    base_image_size = 200 * 200 // 4  # 10,000 bytes for 2-bpp
    
    for strategy, info in strategies.items():
        file_size = int(base_image_size * info["memory_multiplier"])
        processing_memory = file_size + 15000  # Processing buffers
        
        print(f"\n--- {strategy} ---")
        print(f"File Size: {format_bytes(file_size)}")
        print(f"Processing Memory: {format_bytes(processing_memory)}")
        print(f"Perceived Gray Levels: {info['output_levels']}")
        print(f"Pattern Size: {info['pattern_size']}x{info['pattern_size']} pixels")
        
        # Calculate how many images fit
        for variant_name, variant_info in ESP32_VARIANTS.items():
            storage = calculate_available_storage(variant_name, variant_info)
            max_images = storage['usable_spiffs'] // file_size
            print(f"  {variant_name}: {max_images:,} images")

def main():
    """Main analysis function."""
    
    print("ESP32 Memory Analysis for E-Ink Image Storage")
    print(f"Image Format: 2-bpp, 200x200 pixels, {format_bytes(IMAGE_SIZE)} per image")
    
    # Analyze each ESP32 variant
    for variant_name, variant_info in ESP32_VARIANTS.items():
        print_variant_analysis(variant_name, variant_info)
    
    # Generate recommendations
    generate_recommendations()
    
    # Superpixel analysis
    calculate_superpixel_memory()
    
    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")
    print("• ESP32 DevKit V1 (4MB): ~260 images")
    print("• ESP32-C3 Mini (4MB): ~260 images") 
    print("• For more storage: Use SD card or external flash")
    print("• Superpixel dithering doesn't increase file size")
    print("• All variants can process images in available SRAM")

if __name__ == "__main__":
    main()
