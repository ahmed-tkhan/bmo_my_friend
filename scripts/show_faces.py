"""
Host helper to list images on the device and request the device to show each face one by one.
Usage: python show_faces.py COM16 --delay 2
Requires: pyserial
"""
import sys
import time
import argparse

try:
    import serial
except ImportError:
    print("pyserial required: pip install pyserial")
    sys.exit(1)


def send_cmd(ser, cmd, read_response=True, timeout=2):
    ser.write((cmd + "\n").encode('utf-8'))
    if not read_response:
        return []
    lines = []
    end = time.time() + timeout
    while time.time() < end:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line:
            lines.append(line)
            # Stop after LIST end marker
            if line == 'FILELIST_END':
                break
    return lines


def list_files(ser):
    lines = send_cmd(ser, 'LIST', timeout=3)
    files = []
    started = False
    for l in lines:
        if l == 'FILELIST_START':
            started = True
            continue
        if l == 'FILELIST_END':
            break
        if started:
            # Expect lines like: "0: /name.bin"
            parts = l.split(':', 1)
            if len(parts) == 2:
                idx = parts[0].strip()
                name = parts[1].strip()
                files.append((int(idx), name))
    return files


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('port')
    parser.add_argument('--delay', type=float, default=2.0, help='seconds between faces')
    parser.add_argument('--once', action='store_true', help='Show each face once and exit')
    args = parser.parse_args()

    try:
        ser = serial.Serial(args.port, 115200, timeout=1)
    except Exception as e:
        print('Failed to open', args.port, e)
        sys.exit(1)

    # Use EMOTION.LIST API
    lines = send_cmd(ser, 'EMOTION.LIST', timeout=3)
    # parse FILELIST_START..END
    files = []
    started = False
    for l in lines:
        if l == 'FILELIST_START':
            started = True
            continue
        if l == 'FILELIST_END':
            break
        if started:
            parts = l.split(':', 1)
            if len(parts) == 2:
                idx = parts[0].strip()
                name = parts[1].strip()
                files.append((int(idx), name))
    if not files:
        print('No files reported by device')
        ser.close()
        sys.exit(1)

    print('Files on device:')
    for idx, name in files:
        print(idx, name)

    try:
        while True:
            for idx, name in files:
                print('Showing', idx, name)
                send_cmd(ser, f'EMOTION.SETIDX {idx}', read_response=False)
                time.sleep(args.delay)
            if args.once:
                break
    except KeyboardInterrupt:
        print('Interrupted')
    finally:
        ser.close()


if __name__ == '__main__':
    main()
