"""
Minimal serial client helper for communicating with the BMO device.
Usage example:
    from serial_client import SerialClient
    sc = SerialClient('COM16')
    lines = sc.send('EMOTION.LIST')
    sc.send('EMOTION.SETIDX 3', wait_response=False)
"""
import serial
import time

class SerialClient:
    def __init__(self, port, baud=115200, timeout=1):
        self.ser = serial.Serial(port, baud, timeout=timeout)

    def send(self, cmd, wait_response=True, timeout=3):
        # send command with newline
        self.ser.write((cmd + '\n').encode('utf-8'))
        if not wait_response:
            return []
        lines = []
        end = time.time() + timeout
        while time.time() < end:
            line = self.ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                lines.append(line)
                # quick heuristic: if we see FILELIST_END or PONG or OK/ERR, stop early
                if line in ('FILELIST_END', 'PONG') or line.startswith('OK') or line.startswith('ERR'):
                    break
        return lines

    def close(self):
        try:
            self.ser.close()
        except Exception:
            pass

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 3:
        print('Usage: python serial_client.py <port> <COMMAND>')
        sys.exit(1)
    sc = SerialClient(sys.argv[1])
    print('\n'.join(sc.send(' '.join(sys.argv[2:]), timeout=3)))
    sc.close()
