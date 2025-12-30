//! Modbus RTU server implementation

use esp_idf_hal::uart::*;
use esp_idf_hal::gpio::*;
use heapless::Vec;
use log::*;

use crate::error::{FirmwareError, Result};

/// Modbus function codes
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum FunctionCode {
    ReadHoldingRegisters = 0x03,
    WriteSingleRegister = 0x06,
    WriteMultipleRegisters = 0x10,
}

impl From<u8> for FunctionCode {
    fn from(byte: u8) -> Self {
        match byte {
            0x03 => Self::ReadHoldingRegisters,
            0x06 => Self::WriteSingleRegister,
            0x10 => Self::WriteMultipleRegisters,
            _ => Self::ReadHoldingRegisters, // Default
        }
    }
}

/// Modbus request structure
#[derive(Debug)]
pub struct ModbusRequest {
    pub slave_id: u8,
    pub function_code: FunctionCode,
    pub address: u16,
    pub count: u16,
    pub value: u16,
    pub values: Vec<u16, 125>,
}

/// Modbus response structure
#[derive(Debug)]
pub enum ModbusResponse {
    ReadRegisters(Vec<u16, 125>),
    WriteSingleRegister { address: u16, value: u16 },
    WriteMultipleRegisters { address: u16, count: u16 },
}

/// Modbus RTU server
pub struct ModbusServer {
    uart: UartDriver<'static>,
    slave_id: u8,
    rx_buffer: Vec<u8, 256>,
}

impl ModbusServer {
    /// Create new Modbus server
    pub fn new(
        uart: impl Peripheral<P = impl Uart> + 'static,
        tx: impl Peripheral<P = impl OutputPin> + 'static,
        rx: impl Peripheral<P = impl InputPin> + 'static,
        slave_id: u8,
        baudrate: u32,
    ) -> Result<Self> {
        let config = config::Config::new()
            .baudrate(Hertz(baudrate))
            .data_bits(config::DataBits::DataBits8)
            .parity_none()
            .stop_bits(config::StopBits::STOP1);

        let uart = UartDriver::new(
            uart,
            tx,
            rx,
            Option::<AnyIOPin>::None,
            Option::<AnyIOPin>::None,
            &config,
        )
        .map_err(|_| FirmwareError::PeripheralInitFailed)?;

        Ok(Self {
            uart,
            slave_id,
            rx_buffer: Vec::new(),
        })
    }

    /// Poll for incoming Modbus request
    pub fn poll(&mut self) -> Result<Option<ModbusRequest>> {
        let mut buf = [0u8; 1];
        
        match self.uart.read(&mut buf, 0) {
            Ok(1) => {
                self.rx_buffer.push(buf[0])
                    .map_err(|_| FirmwareError::BufferOverflow)?;

                // Check if frame is complete
                if self.is_frame_complete() {
                    let request = self.parse_request()?;
                    self.rx_buffer.clear();
                    return Ok(Some(request));
                }
            }
            _ => {
                // Timeout - clear buffer if too old
                if self.rx_buffer.len() > 0 {
                    // TODO: Add timeout tracking
                    self.rx_buffer.clear();
                }
            }
        }

        Ok(None)
    }

    /// Check if received frame is complete
    fn is_frame_complete(&self) -> bool {
        if self.rx_buffer.len() < 8 {
            return false; // Minimum frame size
        }

        // TODO: Implement proper frame detection with silence interval
        // For now, assume frame is complete after receiving minimum bytes
        true
    }

    /// Parse Modbus request from buffer
    fn parse_request(&self) -> Result<ModbusRequest> {
        if self.rx_buffer.len() < 8 {
            return Err(FirmwareError::ModbusError);
        }

        let slave_id = self.rx_buffer[0];
        if slave_id != self.slave_id {
            return Err(FirmwareError::ModbusError);
        }

        let function_code = FunctionCode::from(self.rx_buffer[1]);
        let address = u16::from_be_bytes([self.rx_buffer[2], self.rx_buffer[3]]);

        // Verify CRC
        if !self.verify_crc() {
            error!("CRC check failed");
            return Err(FirmwareError::ModbusError);
        }

        match function_code {
            FunctionCode::ReadHoldingRegisters => {
                let count = u16::from_be_bytes([self.rx_buffer[4], self.rx_buffer[5]]);
                Ok(ModbusRequest {
                    slave_id,
                    function_code,
                    address,
                    count,
                    value: 0,
                    values: Vec::new(),
                })
            }

            FunctionCode::WriteSingleRegister => {
                let value = u16::from_be_bytes([self.rx_buffer[4], self.rx_buffer[5]]);
                Ok(ModbusRequest {
                    slave_id,
                    function_code,
                    address,
                    count: 1,
                    value,
                    values: Vec::new(),
                })
            }

            Function Code::WriteMultipleRegisters => {
                let count = u16::from_be_bytes([self.rx_buffer[4], self.rx_buffer[5]]);
                let byte_count = self.rx_buffer[6] as usize;
                
                let mut values = Vec::new();
                for i in 0..count {
                    let idx = 7 + (i as usize * 2);
                    let value = u16::from_be_bytes([
                        self.rx_buffer[idx],
                        self.rx_buffer[idx + 1],
                    ]);
                    values.push(value).map_err(|_| FirmwareError::BufferOverflow)?;
                }

                Ok(ModbusRequest {
                    slave_id,
                    function_code,
                    address,
                    count,
                    value: 0,
                    values,
                })
            }
        }
    }

    /// Send Modbus response
    pub fn send_response(&mut self, response: ModbusResponse) -> Result<()> {
        let mut frame: Vec<u8, 256> = Vec::new();

        frame.push(self.slave_id).ok();

        match response {
            ModbusResponse::ReadRegisters(values) => {
                frame.push(FunctionCode::ReadHoldingRegisters as u8).ok();
                frame.push((values.len() * 2) as u8).ok();

                for value in values {
                    frame.push((value >> 8) as u8).ok();
                    frame.push((value & 0xFF) as u8).ok();
                }
            }

            ModbusResponse::WriteSingleRegister { address, value } => {
                frame.push(FunctionCode::WriteSingleRegister as u8).ok();
                frame.push((address >> 8) as u8).ok();
                frame.push((address & 0xFF) as u8).ok();
                frame.push((value >> 8) as u8).ok();
                frame.push((value & 0xFF) as u8).ok();
            }

            ModbusResponse::WriteMultipleRegisters { address, count } => {
                frame.push(FunctionCode::WriteMultipleRegisters as u8).ok();
                frame.push((address >> 8) as u8).ok();
                frame.push((address & 0xFF) as u8).ok();
                frame.push((count >> 8) as u8).ok();
                frame.push((count & 0xFF) as u8).ok();
            }
        }

        // Add CRC
        let crc = Self::calculate_crc(&frame);
        frame.push((crc & 0xFF) as u8).ok();
        frame.push((crc >> 8) as u8).ok();

        // Transmit
        self.uart.write(&frame)
            .map_err(|_| FirmwareError::ModbusError)?;

        Ok(())
    }

    /// Send exception response
    pub fn send_exception(&mut self, function_code: FunctionCode, exception_code: u8) -> Result<()> {
        let mut frame: Vec<u8, 8> = Vec::new();

        frame.push(self.slave_id).ok();
        frame.push((function_code as u8) | 0x80).ok(); // Set high bit for exception
        frame.push(exception_code).ok();

        let crc = Self::calculate_crc(&frame);
        frame.push((crc & 0xFF) as u8).ok();
        frame.push((crc >> 8) as u8).ok();

        self.uart.write(&frame)
            .map_err(|_| FirmwareError::ModbusError)?;

        Ok(())
    }

    /// Verify CRC of received frame
    fn verify_crc(&self) -> bool {
        if self.rx_buffer.len() < 4 {
            return false;
        }

        let data_len = self.rx_buffer.len() - 2;
        let received_crc = u16::from_le_bytes([
            self.rx_buffer[data_len],
            self.rx_buffer[data_len + 1],
        ]);

        let calculated_crc = Self::calculate_crc(&self.rx_buffer[..data_len]);

        received_crc == calculated_crc
    }

    /// Calculate Modbus CRC-16
    fn calculate_crc(data: &[u8]) -> u16 {
        let mut crc: u16 = 0xFFFF;

        for &byte in data {
            crc ^= byte as u16;

            for _ in 0..8 {
                if crc & 0x0001 != 0 {
                    crc >>= 1;
                    crc ^= 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }

        crc
    }
}
