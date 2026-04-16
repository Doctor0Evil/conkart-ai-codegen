// src/generator/tileset.rs
use image::{ImageBuffer, Rgba};
use noise::{NoiseFn, Perlin};
use rand::Rng;
use std::path::PathBuf;

pub struct TilesetConfig {
    pub width: u32,
    pub height: u32,
    pub tile_size: u32,
    pub color_palette: Vec<[u8; 4]>, // RGBA
    pub noise_scale: f64,
}

impl Default for TilesetConfig {
    fn default() -> Self {
        Self {
            width: 256,
            height: 256,
            tile_size: 16,
            color_palette: vec![
                [34, 139, 34, 255],   // Forest Green
                [139, 69, 19, 255],   // Brown
                [105, 105, 105, 255], // Dim Gray
            ],
            noise_scale: 0.05,
        }
    }
}

pub fn generate_tileset_image(config: &TilesetConfig, output_path: &PathBuf) -> Result<(), Box<dyn std::error::Error>> {
    let mut img = ImageBuffer::new(config.width, config.height);
    let perlin = Perlin::new(rand::thread_rng().gen());

    for (x, y, pixel) in img.enumerate_pixels_mut() {
        // Generate a procedural value using Perlin noise
        let noise_val = perlin.get([x as f64 * config.noise_scale, y as f64 * config.noise_scale]);
        let normalized = (noise_val + 1.0) / 2.0; // Map from [-1, 1] to [0, 1]
        
        // Select a color from the palette based on the noise value
        let palette_index = (normalized * (config.color_palette.len() - 1) as f64).round() as usize;
        let color = config.color_palette[palette_index];
        
        *pixel = Rgba(color);
    }
    
    img.save(output_path)?;
    Ok(())
}
