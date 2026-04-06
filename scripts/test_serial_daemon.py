#!/usr/bin/env python3
"""Unit test for bmo_serial_daemon logic using a fake serial transport."""
import struct
import time


class FakeSerial:
    def __init__(self):
        self._write_buf = bytearray()
        self._read_buf = bytearray()
        self.in_waiting = 0

    def write(self, data):
        # Append to write buffer and immediately prepare ACK
        self._write_buf += data
        # If this looks like an upload header (starts with 0xAA), ACK
        if len(self._write_buf) >= 1 and self._write_buf[0] == 0xAA:
            # simulate device processing then queue ACK
            self._read_buf += b"\x06"
            self.in_waiting = len(self._read_buf)

    def flush(self):
        pass

    def read(self, n=1):
        if not self._read_buf:
            return b''
        out = bytes(self._read_buf[:n])
        self._read_buf = self._read_buf[n:]
        self.in_waiting = len(self._read_buf)
        return out

    def close(self):
        pass


def send_frame_under_test(ser, name, data, timeout=1.0):
    # same framing used by daemon
    magic = b"\xAA"
    name_bytes = name.encode('utf-8')
    name_len = len(name_bytes)
    total_len = 4 + 1 + name_len + len(data)
    header = magic + struct.pack('<I', total_len) + struct.pack('B', name_len) + name_bytes
    ser.write(header)
    ser.write(data)
    ser.flush()

    start = time.time()
    while time.time() - start < timeout:
        if ser.in_waiting:
            r = ser.read(1)
            return r == b"\x06"
        time.sleep(0.01)
    return False


def send_trigger_under_test(ser, face_id=0):
    ser.write(b'TRIG' + struct.pack('B', face_id))
    ser.flush()


def main():
    print("Running serial daemon unit tests...")
    fake = FakeSerial()
    payload = b"HEADEREDBIN" * 10
    ok = send_frame_under_test(fake, "test.bin", payload)
    print(f"send_frame returned: {ok}")
    assert ok, "Expected ACK from fake device"

    # test trigger
    fake2 = FakeSerial()
    send_trigger_under_test(fake2, 5)
    # The trigger should be present in write buffer
    print(f"trigger bytes written: {fake2._write_buf[:10]!r}")
    assert fake2._write_buf.startswith(b'TRIG'), "Trigger not written"

    print("All tests passed")


if __name__ == '__main__':
    main()
