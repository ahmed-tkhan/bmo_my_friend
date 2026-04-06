#!/usr/bin/env python3
"""
Minimal BMO serial daemon
- Upload a headered .bin file to device over a simple framed protocol
- Trigger display by sending a short command after upload

Usage examples:
  python bmo_serial_daemon.py --port COM7 --file ../data/smile_region.bin

This is intentionally small: uses pyserial for transport. The device
should implement a matching simple receive handler that accepts:
  [0xAA][len(4 bytes little)][name_len][name bytes][payload bytes]
and replies with single-byte ACK (0x06) or NAK (0x15).

If the device does not implement file upload, the daemon will send a
simple trigger sequence: `TRIG` followed by face id (1 byte).
"""

import argparse
import os
import struct
import time
import serial

ACK = b"\x06"
NAK = b"\x15"


def send_frame(ser, name, data, timeout=5.0):
    """Send a simple framed payload: magic + length + name + data"""
    magic = b"\xAA"
    name_bytes = name.encode('utf-8')
    name_len = len(name_bytes)
    total_len = 4 + 1 + name_len + len(data)  # placeholder length field semantics
    header = magic + struct.pack('<I', total_len) + struct.pack('B', name_len) + name_bytes
    ser.write(header)
    ser.write(data)
    ser.flush()

    # wait for ACK/NAK
    start = time.time()
    while time.time() - start < timeout:
        if ser.in_waiting:
            r = ser.read(1)
            return r == ACK
        time.sleep(0.01)
    return False


def send_trigger(ser, face_id=0):
    # Simple trigger: ASCII 'TRIG' + face_id byte
    ser.write(b'TRIG' + struct.pack('B', face_id))
    ser.flush()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--port', required=True, help='Serial port (e.g., COM7 or /dev/ttyUSB0)')
    parser.add_argument('--baud', type=int, default=115200)
    parser.add_argument('--file', help='Path to headered .bin file to upload')
    parser.add_argument('--name', help='Remote filename to store as (optional)')
    parser.add_argument('--trigger-only', action='store_true', help='Only send trigger')
    parser.add_argument('--face-id', type=int, default=0, help='Face ID to trigger')
    args = parser.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=0.1)
    print(f"Opened {args.port} @ {args.baud}")

    try:
        if args.trigger_only:
            print(f"Sending trigger face_id={args.face_id}")
            send_trigger(ser, args.face_id)
            print("Trigger sent")
            return

        if not args.file:
            print("No file provided; use --trigger-only or --file")
            return

        path = os.path.expanduser(args.file)
        if not os.path.exists(path):
            print(f"File not found: {path}")
            return

        with open(path, 'rb') as f:
            payload = f.read()

        remote_name = args.name or os.path.basename(path)
        print(f"Uploading {path} as {remote_name} ({len(payload)} bytes)")
        ok = send_frame(ser, remote_name, payload)
        if ok:
            print("Upload ACK received")
            # Optionally send a trigger to display the uploaded file
            print(f"Sending display trigger (face_id={args.face_id})")
            send_trigger(ser, args.face_id)
        else:
            print("No ACK received from device; upload may have failed")

    finally:
        ser.close()


if __name__ == '__main__':
    main()
