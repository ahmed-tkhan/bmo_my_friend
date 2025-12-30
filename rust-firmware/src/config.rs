//! System configuration module

use serde::{Deserialize, Serialize};

/// System-wide configuration
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SystemConfig {
    /// Modbus slave ID (1-247)
    pub modbus_slave_id: u8,
    
    /// Modbus baudrate
    pub modbus_baudrate: u32,
    
    /// Default face on startup
    pub startup_face_id: u8,
    
    /// Partial refresh threshold before full refresh
    pub partial_refresh_threshold: u16,
    
    /// Watchdog timeout (seconds)
    pub watchdog_timeout: u32,
}

impl Default for SystemConfig {
    fn default() -> Self {
        Self {
            modbus_slave_id: 1,
            modbus_baudrate: 9600,
            startup_face_id: 0,
            partial_refresh_threshold: 10,
            watchdog_timeout: 30,
        }
    }
}

impl SystemConfig {
    /// Load configuration from flash (or use default)
    pub fn load() -> crate::error::Result<Self> {
        // TODO: Load from NVS (Non-Volatile Storage)
        // For now, return default
        Ok(Self::default())
    }
    
    /// Save configuration to flash
    pub fn save(&self) -> crate::error::Result<()> {
        // TODO: Save to NVS
        Ok(())
    }
}
