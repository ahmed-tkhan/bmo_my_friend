"""
Simple script to send a serial command to the device to trigger full-refresh cycles.
Usage: python trigger_full_refresh.py COM3 50

Requires: pyserial (pip install pyserial)
"""
import sys
import time

try:
    import serial
except ImportError:
    print("pyserial is required. Install with: pip install pyserial")
    sys.exit(1)


def main():
    if len(sys.argv) < 3:
        print("Usage: python trigger_full_refresh.py <port> <count>")
        sys.exit(1)
    port = sys.argv[1]
    try:
        cnt = int(sys.argv[2])
    except ValueError:
        print("Invalid count")
        sys.exit(1)

    try:
        ser = serial.Serial(port, 115200, timeout=1)
    except Exception as e:
        print(f"Failed to open serial port {port}: {e}")
        sys.exit(1)

    cmd = f"BURN {cnt}\n"
    print(f"Sending: {cmd.strip()} to {port}")
    ser.write(cmd.encode('utf-8'))
    # Give device a bit to respond and run
    time.sleep(0.5)
    # Read any immediate responses for a short while
    end = time.time() + 3
    while time.time() < end:
        line = ser.readline().decode('utf-8', errors='ignore')
        if line:
            print(line.strip())
    ser.close()

if __name__ == '__main__':
    main()
