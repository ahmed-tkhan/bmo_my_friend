#!/usr/bin/env python3
"""
BMO Modbus Test Client
Tests the Modbus interface of the BMO firmware
"""

from pymodbus.client import ModbusSerialClient
import time
import sys

# Modbus register addresses
REG_FACE_ID = 0x0000
REG_ANIMATION_MODE = 0x0001
REG_REFRESH_INTERVAL = 0x0002
REG_SYSTEM_STATUS = 0x0004
REG_ERROR_CODE = 0x0005
REG_PARTIAL_REFRESH_CNT = 0x0006
REG_FULL_REFRESH_TRIG = 0x0007

# Face IDs
FACE_NEUTRAL = 0
FACE_HAPPY = 1
FACE_SAD = 2
FACE_ANGRY = 3
FACE_SURPRISED = 4
FACE_CONFUSED = 5
FACE_SLEEPING = 6
FACE_EXCITED = 7

class BMOModbusClient:
    def __init__(self, port='COM7', baudrate=9600, slave_id=1):
        self.client = ModbusSerialClient(
            port=port,
            baudrate=baudrate,
            parity='N',
            stopbits=1,
            bytesize=8,
            timeout=1
        )
        self.slave_id = slave_id
        
    def connect(self):
        """Connect to BMO device"""
        if self.client.connect():
            print(f"✓ Connected to BMO on {self.client.port}")
            return True
        else:
            print(f"✗ Failed to connect to {self.client.port}")
            return False
    
    def disconnect(self):
        """Disconnect from BMO device"""
        self.client.close()
        print("✓ Disconnected")
    
    def read_registers(self, address, count=1):
        """Read holding registers"""
        result = self.client.read_holding_registers(
            address=address,
            count=count,
            unit=self.slave_id
        )
        
        if result.isError():
            print(f"✗ Error reading register 0x{address:04X}: {result}")
            return None
        
        return result.registers
    
    def write_register(self, address, value):
        """Write single register"""
        result = self.client.write_register(
            address=address,
            value=value,
            unit=self.slave_id
        )
        
        if result.isError():
            print(f"✗ Error writing register 0x{address:04X}: {result}")
            return False
        
        return True
    
    def get_face_id(self):
        """Get current face ID"""
        regs = self.read_registers(REG_FACE_ID, 1)
        return regs[0] if regs else None
    
    def set_face_id(self, face_id):
        """Set face ID"""
        if face_id < 0 or face_id > 15:
            print(f"✗ Invalid face ID: {face_id} (must be 0-15)")
            return False
        
        if self.write_register(REG_FACE_ID, face_id):
            print(f"✓ Face changed to ID {face_id}")
            return True
        return False
    
    def get_system_status(self):
        """Get system status flags"""
        regs = self.read_registers(REG_SYSTEM_STATUS, 1)
        if not regs:
            return None
        
        status = regs[0]
        return {
            'ready': bool(status & (1 << 0)),
            'busy': bool(status & (1 << 1)),
            'error': bool(status & (1 << 2)),
        }
    
    def get_error_code(self):
        """Get last error code"""
        regs = self.read_registers(REG_ERROR_CODE, 1)
        return regs[0] if regs else None
    
    def trigger_full_refresh(self):
        """Trigger full display refresh"""
        if self.write_register(REG_FULL_REFRESH_TRIG, 1):
            print("✓ Full refresh triggered")
            return True
        return False
    
    def set_animation_mode(self, mode):
        """Set animation mode (0=Static, 1=Cycle, 2=Random)"""
        if self.write_register(REG_ANIMATION_MODE, mode):
            mode_names = ['Static', 'Cycle', 'Random']
            print(f"✓ Animation mode set to: {mode_names[mode]}")
            return True
        return False
    
    def print_status(self):
        """Print current system status"""
        print("\n" + "="*50)
        print("BMO SYSTEM STATUS")
        print("="*50)
        
        face_id = self.get_face_id()
        print(f"Current Face ID:    {face_id}")
        
        status = self.get_system_status()
        if status:
            print(f"Ready:              {status['ready']}")
            print(f"Busy:               {status['busy']}")
            print(f"Error:              {status['error']}")
        
        error_code = self.get_error_code()
        print(f"Last Error Code:    {error_code}")
        
        print("="*50 + "\n")


def test_face_cycle(bmo):
    """Test cycling through all faces"""
    print("\n=== Test: Face Cycle ===")
    
    faces = [
        (FACE_NEUTRAL, "Neutral"),
        (FACE_HAPPY, "Happy"),
        (FACE_SAD, "Sad"),
        (FACE_ANGRY, "Angry"),
        (FACE_SURPRISED, "Surprised"),
        (FACE_CONFUSED, "Confused"),
        (FACE_SLEEPING, "Sleeping"),
        (FACE_EXCITED, "Excited"),
    ]
    
    for face_id, name in faces:
        print(f"\nShowing {name} face...")
        bmo.set_face_id(face_id)
        time.sleep(2)  # Wait 2 seconds between faces
    
    print("\n✓ Face cycle complete")


def test_rapid_switching(bmo):
    """Test rapid face switching"""
    print("\n=== Test: Rapid Face Switching ===")
    
    faces = [FACE_HAPPY, FACE_SURPRISED, FACE_EXCITED, FACE_HAPPY]
    
    for _ in range(3):  # Repeat 3 times
        for face_id in faces:
            bmo.set_face_id(face_id)
            time.sleep(0.5)  # Fast switching
    
    print("✓ Rapid switching test complete")


def test_full_refresh(bmo):
    """Test full refresh trigger"""
    print("\n=== Test: Full Refresh ===")
    
    bmo.trigger_full_refresh()
    time.sleep(2)
    
    print("✓ Full refresh test complete")


def interactive_mode(bmo):
    """Interactive mode for manual testing"""
    print("\n" + "="*50)
    print("INTERACTIVE MODE")
    print("="*50)
    print("Commands:")
    print("  0-7   : Show face by ID")
    print("  s     : Show status")
    print("  f     : Full refresh")
    print("  q     : Quit")
    print("="*50 + "\n")
    
    while True:
        try:
            cmd = input("BMO> ").strip().lower()
            
            if cmd == 'q':
                break
            elif cmd == 's':
                bmo.print_status()
            elif cmd == 'f':
                bmo.trigger_full_refresh()
            elif cmd.isdigit():
                face_id = int(cmd)
                bmo.set_face_id(face_id)
            else:
                print("Unknown command")
        
        except KeyboardInterrupt:
            print("\n")
            break
        except Exception as e:
            print(f"Error: {e}")


def main():
    # Parse command line arguments
    port = sys.argv[1] if len(sys.argv) > 1 else 'COM7'
    
    # Create client
    bmo = BMOModbusClient(port=port, baudrate=9600, slave_id=1)
    
    # Connect
    if not bmo.connect():
        sys.exit(1)
    
    try:
        # Show initial status
        bmo.print_status()
        
        # Run tests
        test_face_cycle(bmo)
        test_rapid_switching(bmo)
        test_full_refresh(bmo)
        
        # Enter interactive mode
        interactive_mode(bmo)
    
    finally:
        # Cleanup
        bmo.disconnect()


if __name__ == '__main__':
    main()
