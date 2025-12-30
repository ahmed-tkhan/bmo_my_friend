//! Error types for the firmware

use core::fmt;

pub type Result<T> = core::result::Result<T, FirmwareError>;

/// Firmware error types
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FirmwareError {
    /// Display hardware error
    DisplayError,
    
    /// SPI communication timeout
    SpiTimeout,
    
    /// Invalid face ID requested
    InvalidFaceId(u16),
    
    /// Face asset not found in filesystem
    AssetNotFound,
    
    /// File system error
    FileSystemError,
    
    /// Memory exhausted
    OutOfMemory,
    
    /// Buffer overflow
    BufferOverflow,
    
    /// Modbus protocol error
    ModbusError,
    
    /// Modbus illegal function code
    ModbusIllegalFunction,
    
    /// Modbus illegal data address
    ModbusIllegalAddress,
    
    /// Modbus illegal data value
    ModbusIllegalValue,
    
    /// Peripheral initialization failed
    PeripheralInitFailed,
    
    /// Generic hardware fault
    HardwareFault,
}

impl fmt::Display for FirmwareError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::DisplayError => write!(f, "Display hardware error"),
            Self::SpiTimeout => write!(f, "SPI communication timeout"),
            Self::InvalidFaceId(id) => write!(f, "Invalid face ID: {}", id),
            Self::AssetNotFound => write!(f, "Face asset not found"),
            Self::FileSystemError => write!(f, "File system error"),
            Self::OutOfMemory => write!(f, "Out of memory"),
            Self::BufferOverflow => write!(f, "Buffer overflow"),
            Self::ModbusError => write!(f, "Modbus protocol error"),
            Self::ModbusIllegalFunction => write!(f, "Modbus illegal function"),
            Self::ModbusIllegalAddress => write!(f, "Modbus illegal address"),
            Self::ModbusIllegalValue => write!(f, "Modbus illegal value"),
            Self::PeripheralInitFailed => write!(f, "Peripheral init failed"),
            Self::HardwareFault => write!(f, "Hardware fault"),
        }
    }
}

impl From<FirmwareError> for u8 {
    /// Convert to Modbus exception code
    fn from(err: FirmwareError) -> u8 {
        match err {
            FirmwareError::ModbusIllegalFunction => 0x01,
            FirmwareError::ModbusIllegalAddress => 0x02,
            FirmwareError::ModbusIllegalValue => 0x03,
            _ => 0x04, // Slave device failure
        }
    }
}

// Global error tracking for status register
static mut LAST_ERROR: Option<FirmwareError> = None;

pub fn set_last_error(error: FirmwareError) {
    unsafe {
        LAST_ERROR = Some(error);
    }
}

pub fn get_last_error_code() -> u16 {
    unsafe {
        LAST_ERROR.map(|e| e as u16).unwrap_or(0)
    }
}

pub fn has_error() -> bool {
    unsafe { LAST_ERROR.is_some() }
}

pub fn clear_error() {
    unsafe {
        LAST_ERROR = None;
    }
}
