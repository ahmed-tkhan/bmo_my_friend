//! Display controller module - Hardware abstraction for e-paper display

use esp_idf_hal::gpio::*;
use esp_idf_hal::delay::FreeRtos;
use log::*;

use crate::error::{FirmwareError, Result};

/// Display controller for GDEH0154D67 e-paper display
pub struct DisplayController {
    dc: PinDriver<'static, AnyOutputPin, Output>,
    cs: PinDriver<'static, AnyOutputPin, Output>,
    rst: PinDriver<'static, AnyOutputPin, Output>,
    busy: PinDriver<'static, AnyInputPin, Input>,
    sck: PinDriver<'static, AnyOutputPin, Output>,
    mosi: PinDriver<'static, AnyOutputPin, Output>,
    partial_refresh_count: u16,
}

impl DisplayController {
    /// Create new display controller
    pub fn new(
        dc: impl Peripheral<P = impl OutputPin> + 'static,
        cs: impl Peripheral<P = impl OutputPin> + 'static,
        rst: impl Peripheral<P = impl OutputPin> + 'static,
        busy: impl Peripheral<P = impl InputPin> + 'static,
        sck: impl Peripheral<P = impl OutputPin> + 'static,
        mosi: impl Peripheral<P = impl OutputPin> + 'static,
    ) -> Result<Self> {
        let dc = PinDriver::output(dc.into_ref())
            .map_err(|_| FirmwareError::PeripheralInitFailed)?;
        let cs = PinDriver::output(cs.into_ref())
            .map_err(|_| FirmwareError::PeripheralInitFailed)?;
        let rst = PinDriver::output(rst.into_ref())
            .map_err(|_| FirmwareError::PeripheralInitFailed)?;
        let busy = PinDriver::input(busy.into_ref())
            .map_err(|_| FirmwareError::PeripheralInitFailed)?;
        let sck = PinDriver::output(sck.into_ref())
            .map_err(|_| FirmwareError::PeripheralInitFailed)?;
        let mosi = PinDriver::output(mosi.into_ref())
            .map_err(|_| FirmwareError::PeripheralInitFailed)?;

        let mut controller = Self {
            dc,
            cs,
            rst,
            busy,
            sck,
            mosi,
            partial_refresh_count: 0,
        };

        controller.init()?;
        Ok(controller)
    }

    /// Initialize display hardware
    fn init(&mut self) -> Result<()> {
        info!("Initializing e-paper display...");

        // Hardware reset
        self.rst.set_high().ok();
        FreeRtos::delay_ms(10);
        self.rst.set_low().ok();
        FreeRtos::delay_ms(10);
        self.rst.set_high().ok();
        FreeRtos::delay_ms(10);

        self.wait_busy();

        // Software reset
        self.write_command(0x12)?;
        self.wait_busy();

        // Driver output control
        self.write_command(0x01)?;
        self.write_data(0xC7)?;
        self.write_data(0x00)?;
        self.write_data(0x00)?;

        // Data entry mode
        self.write_command(0x11)?;
        self.write_data(0x01)?;

        // Set RAM X address
        self.write_command(0x44)?;
        self.write_data(0x00)?;
        self.write_data(0x18)?;

        // Set RAM Y address
        self.write_command(0x45)?;
        self.write_data(0xC7)?;
        self.write_data(0x00)?;
        self.write_data(0x00)?;
        self.write_data(0x00)?;

        // Border waveform
        self.write_command(0x3C)?;
        self.write_data(0x05)?;

        // Temperature sensor
        self.write_command(0x18)?;
        self.write_data(0x80)?;

        // Load LUT for monochrome
        self.load_lut(&LUT_FULL_UPDATE)?;

        self.wait_busy();

        info!("Display initialization complete");
        Ok(())
    }

    /// Show a face on the display
    pub fn show_face(&mut self, face_data: &FaceData) -> Result<()> {
        info!("Displaying face at ({}, {}), size {}x{}", 
            face_data.x, face_data.y, face_data.width, face_data.height);

        // Set window
        self.set_window(face_data.x, face_data.y, face_data.width, face_data.height)?;

        // Write image data
        self.write_command(0x24)?;
        for &byte in face_data.data.iter() {
            self.write_data(byte)?;
        }

        // Trigger partial refresh
        self.partial_refresh()?;
        self.partial_refresh_count += 1;

        // Periodic full refresh to prevent ghosting
        if self.partial_refresh_count >= 10 {
            info!("Threshold reached, performing full refresh");
            self.full_refresh()?;
            self.partial_refresh_count = 0;
        }

        Ok(())
    }

    /// Trigger full display refresh
    pub fn full_refresh(&mut self) -> Result<()> {
        self.load_lut(&LUT_FULL_UPDATE)?;
        self.write_command(0x22)?;
        self.write_data(0xF7)?;
        self.write_command(0x20)?;
        self.wait_busy();
        Ok(())
    }

    /// Trigger partial display refresh
    fn partial_refresh(&mut self) -> Result<()> {
        self.load_lut(&LUT_PARTIAL_UPDATE)?;
        self.write_command(0x22)?;
        self.write_data(0xFF)?;
        self.write_command(0x20)?;
        FreeRtos::delay_ms(200); // Fixed delay instead of BUSY polling
        Ok(())
    }

    /// Set display window for partial update
    fn set_window(&mut self, x: u16, y: u16, width: u16, height: u16) -> Result<()> {
        let x_start = (x / 8) as u8;
        let x_end = ((x + width) / 8 - 1) as u8;

        self.write_command(0x44)?;
        self.write_data(x_start)?;
        self.write_data(x_end)?;

        self.write_command(0x45)?;
        self.write_data((y & 0xFF) as u8)?;
        self.write_data((y >> 8) as u8)?;
        self.write_data(((y + height - 1) & 0xFF) as u8)?;
        self.write_data(((y + height - 1) >> 8) as u8)?;

        self.write_command(0x4E)?;
        self.write_data(x_start)?;
        self.write_command(0x4F)?;
        self.write_data((y & 0xFF) as u8)?;
        self.write_data((y >> 8) as u8)?;

        Ok(())
    }

    /// Load lookup table
    fn load_lut(&mut self, lut: &[u8]) -> Result<()> {
        self.write_command(0x32)?;
        for &byte in lut {
            self.write_data(byte)?;
        }
        Ok(())
    }

    /// Write command byte
    fn write_command(&mut self, cmd: u8) -> Result<()> {
        self.cs.set_low().ok();
        self.dc.set_low().ok();
        self.spi_write(cmd);
        self.cs.set_high().ok();
        Ok(())
    }

    /// Write data byte
    fn write_data(&mut self, data: u8) -> Result<()> {
        self.cs.set_low().ok();
        self.dc.set_high().ok();
        self.spi_write(data);
        self.cs.set_high().ok();
        Ok(())
    }

    /// Bit-bang SPI write
    fn spi_write(&mut self, mut value: u8) {
        for _ in 0..8 {
            self.sck.set_low().ok();
            
            if value & 0x80 != 0 {
                self.mosi.set_high().ok();
            } else {
                self.mosi.set_low().ok();
            }
            
            value <<= 1;
            self.sck.set_high().ok();
        }
    }

    /// Wait for display to be ready (with timeout)
    fn wait_busy(&mut self) {
        // Fixed delay workaround since BUSY pin may not be reliable
        FreeRtos::delay_ms(200);
    }
}

/// Face data structure
pub struct FaceData {
    pub x: u16,
    pub y: u16,
    pub width: u16,
    pub height: u16,
    pub data: heapless::Vec<u8, 5000>,
}

/// LUT for full update
const LUT_FULL_UPDATE: [u8; 30] = [
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00,
];

/// LUT for partial update
const LUT_PARTIAL_UPDATE: [u8; 30] = [
    0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
];

// Global busy state for status register
static mut IS_BUSY: bool = false;

pub fn is_busy() -> bool {
    unsafe { IS_BUSY }
}
