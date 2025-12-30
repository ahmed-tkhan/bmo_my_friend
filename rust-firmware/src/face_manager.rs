//! Face asset manager - loads and manages BMO face expressions

use std::fs::File;
use std::io::Read;
use std::path::PathBuf;
use heapless::Vec;
use log::*;

use crate::display::FaceData;
use crate::error::{FirmwareError, Result};

/// Animation mode
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AnimationMode {
    Static = 0,
    Cycle = 1,
    Random = 2,
}

impl From<u16> for AnimationMode {
    fn from(value: u16) -> Self {
        match value {
            1 => Self::Cycle,
            2 => Self::Random,
            _ => Self::Static,
        }
    }
}

/// Face manager
pub struct FaceManager {
    base_path: PathBuf,
    available_faces: Vec<u8, 16>,
    current_face_id: u8,
    animation_mode: AnimationMode,
    refresh_interval: u16,
}

impl FaceManager {
    /// Create new face manager
    pub fn new(base_path: &str) -> Result<Self> {
        Ok(Self {
            base_path: PathBuf::from(base_path),
            available_faces: Vec::new(),
            current_face_id: 0,
            animation_mode: AnimationMode::Static,
            refresh_interval: 1000,
        })
    }

    /// Scan filesystem for available face assets
    pub fn scan_faces(&mut self) -> Result<()> {
        info!("Scanning for face assets...");

        // TODO: Implement directory scanning with ESP-IDF VFS
        // For now, assume faces 0-7 are available
        for i in 0..8 {
            self.available_faces.push(i)
                .map_err(|_| FirmwareError::BufferOverflow)?;
        }

        info!("Found {} face assets", self.available_faces.len());
        Ok(())
    }

    /// Load face data from filesystem
    pub fn load_face(&self, face_id: u8) -> Result<FaceData> {
        if !self.available_faces.contains(&face_id) {
            error!("Face ID {} not found", face_id);
            return Err(FirmwareError::InvalidFaceId(face_id as u16));
        }

        let filename = format!("{}_{}_*.bin", self.base_path.display(), face_id);
        info!("Loading face: {}", filename);

        // TODO: Implement actual file loading with ESP-IDF VFS
        // For now, return dummy data
        let mut data = Vec::new();
        for _ in 0..1680 {
            data.push(0xFF).ok(); // White pixels
        }

        Ok(FaceData {
            x: 20,
            y: 58,
            width: 160,
            height: 84,
            data,
        })
    }

    /// Get number of available faces
    pub fn face_count(&self) -> usize {
        self.available_faces.len()
    }

    /// Update animations (called from main loop)
    pub fn update_animations(&mut self, display: &mut crate::display::DisplayController) -> Result<()> {
        // TODO: Implement animation logic based on mode and interval
        Ok(())
    }

    /// Set animation mode
    pub fn set_animation_mode(&mut self, mode: AnimationMode) -> Result<()> {
        self.animation_mode = mode;
        Ok(())
    }

    /// Set refresh interval
    pub fn set_refresh_interval(&mut self, interval_ms: u16) -> Result<()> {
        self.refresh_interval = interval_ms;
        Ok(())
    }
}

// Global state for Modbus registers
static mut CURRENT_FACE_ID: u8 = 0;
static mut ANIMATION_MODE: AnimationMode = AnimationMode::Static;
static mut REFRESH_INTERVAL: u16 = 1000;

pub fn get_current_face_id() -> u16 {
    unsafe { CURRENT_FACE_ID as u16 }
}

pub fn set_current_face_id(id: u16) {
    unsafe { CURRENT_FACE_ID = id as u8; }
}

pub fn get_animation_mode() -> AnimationMode {
    unsafe { ANIMATION_MODE }
}

pub fn get_refresh_interval() -> u16 {
    unsafe { REFRESH_INTERVAL }
}
