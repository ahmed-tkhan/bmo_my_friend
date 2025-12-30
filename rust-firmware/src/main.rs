//! # BMO Display Firmware - Modbus Edition
//! 
//! A modular, Modbus-enabled firmware for controlling BMO e-paper display expressions.
//! Built with Rust for memory safety and reliability on ESP32-C3.

#![cfg_attr(not(feature = "std"), no_std)]

use esp_idf_hal::prelude::*;
use log::*;

mod config;
mod display;
mod modbus;
mod face_manager;
mod error;

use config::SystemConfig;
use display::DisplayController;
use modbus::ModbusServer;
use face_manager::FaceManager;
use error::Result;

/// Main application entry point
fn main() -> Result<()> {
    // Initialize ESP-IDF services
    esp_idf_sys::link_patches();
    esp_idf_svc::log::EspLogger::initialize_default();

    info!("BMO Firmware starting...");
    info!("Version: {}", env!("CARGO_PKG_VERSION"));

    // Load system configuration
    let config = SystemConfig::load()?;
    info!("Configuration loaded: {:?}", config);

    // Initialize peripherals
    let peripherals = Peripherals::take()
        .ok_or(error::FirmwareError::PeripheralInitFailed)?;

    // Initialize display controller
    let mut display = DisplayController::new(
        peripherals.gpio2,  // DC
        peripherals.gpio3,  // CS
        peripherals.gpio0,  // RST
        peripherals.gpio1,  // BUSY
        peripherals.gpio8,  // SCK
        peripherals.gpio10, // MOSI
    )?;

    info!("Display controller initialized");

    // Initialize face manager with SPIFFS
    let mut face_manager = FaceManager::new("/spiffs")?;
    face_manager.scan_faces()?;
    info!("Found {} face assets", face_manager.face_count());

    // Initialize Modbus server on UART1
    let modbus_uart = peripherals.uart1;
    let tx_pin = peripherals.gpio6;
    let rx_pin = peripherals.gpio7;
    
    let mut modbus = ModbusServer::new(
        modbus_uart,
        tx_pin,
        rx_pin,
        config.modbus_slave_id,
        config.modbus_baudrate,
    )?;

    info!("Modbus server initialized (Slave ID: {})", config.modbus_slave_id);

    // Display startup face
    display.show_face(&face_manager.load_face(0)?)?;
    info!("Startup face displayed");

    // Main application loop
    loop {
        // Process incoming Modbus requests
        if let Some(request) = modbus.poll()? {
            match handle_modbus_request(&request, &mut display, &mut face_manager) {
                Ok(response) => {
                    modbus.send_response(response)?;
                }
                Err(e) => {
                    error!("Modbus request failed: {:?}", e);
                    modbus.send_exception(request.function_code, e.into())?;
                }
            }
        }

        // Background tasks
        face_manager.update_animations(&mut display)?;
        
        // Yield to watchdog
        esp_idf_hal::delay::FreeRtos::delay_ms(1);
    }
}

/// Handle Modbus request and execute corresponding action
fn handle_modbus_request(
    request: &modbus::ModbusRequest,
    display: &mut DisplayController,
    face_manager: &mut FaceManager,
) -> Result<modbus::ModbusResponse> {
    use modbus::{FunctionCode, ModbusRequest, ModbusResponse};

    match request.function_code {
        // Read Holding Registers
        FunctionCode::ReadHoldingRegisters => {
            let values = read_registers(request.address, request.count)?;
            Ok(ModbusResponse::ReadRegisters(values))
        }

        // Write Single Register
        FunctionCode::WriteSingleRegister => {
            write_register(request.address, request.value, display, face_manager)?;
            Ok(ModbusResponse::WriteSingleRegister {
                address: request.address,
                value: request.value,
            })
        }

        // Write Multiple Registers
        FunctionCode::WriteMultipleRegisters => {
            for (i, &value) in request.values.iter().enumerate() {
                write_register(
                    request.address + i as u16,
                    value,
                    display,
                    face_manager,
                )?;
            }
            Ok(ModbusResponse::WriteMultipleRegisters {
                address: request.address,
                count: request.count,
            })
        }

        _ => Err(error::FirmwareError::ModbusIllegalFunction),
    }
}

/// Read register values
fn read_registers(address: u16, count: u16) -> Result<heapless::Vec<u16, 125>> {
    use heapless::Vec;
    
    let mut values = Vec::new();
    
    for i in 0..count {
        let reg_addr = address + i;
        let value = match reg_addr {
            0x0000 => face_manager::get_current_face_id(),
            0x0001 => face_manager::get_animation_mode() as u16,
            0x0002 => face_manager::get_refresh_interval(),
            0x0004 => get_system_status(),
            0x0005 => error::get_last_error_code(),
            _ => 0, // Reserved or unimplemented
        };
        values.push(value).map_err(|_| error::FirmwareError::BufferOverflow)?;
    }
    
    Ok(values)
}

/// Write to register and execute action
fn write_register(
    address: u16,
    value: u16,
    display: &mut DisplayController,
    face_manager: &mut FaceManager,
) -> Result<()> {
    match address {
        // FACE_ID register
        0x0000 => {
            if value > 15 {
                return Err(error::FirmwareError::InvalidFaceId(value));
            }
            let face_data = face_manager.load_face(value as u8)?;
            display.show_face(&face_data)?;
            face_manager::set_current_face_id(value);
            info!("Face changed to ID {}", value);
        }

        // ANIMATION_MODE register
        0x0001 => {
            face_manager.set_animation_mode(value.into())?;
            info!("Animation mode set to {}", value);
        }

        // REFRESH_INTERVAL register
        0x0002 => {
            face_manager.set_refresh_interval(value)?;
            info!("Refresh interval set to {}ms", value);
        }

        // FULL_REFRESH_TRIG register
        0x0007 => {
            if value == 1 {
                display.full_refresh()?;
                info!("Full refresh triggered");
            }
        }

        _ => {
            warn!("Write to unimplemented register 0x{:04X}", address);
        }
    }

    Ok(())
}

/// Get system status flags
fn get_system_status() -> u16 {
    let mut status = 0u16;
    
    // Bit 0: Ready
    status |= 1 << 0;
    
    // Bit 1: Busy (display updating)
    if display::is_busy() {
        status |= 1 << 1;
    }
    
    // Bit 2: Error
    if error::has_error() {
        status |= 1 << 2;
    }
    
    status
}
