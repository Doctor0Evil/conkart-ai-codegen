// Filename: conkart_grid_ai.rs
// Destination: crates/conkart_grid_ai/src/main.rs
use std::collections::{BTreeMap, HashSet};
use std::fs;
use std::path::PathBuf;
use clap::Parser;
use serde::{Deserialize, Serialize};
use anyhow::{Context, Result};

#[derive(Debug, Parser)]
#[command(name = "conkart-grid-ai", about = "Conker AI Grid & Tileset Validator + Prompt Generator")]
struct Cli {
    #[arg(long)]
    grid_path: PathBuf,
    #[arg(long)]
    tileset_path: PathBuf,
    #[arg(long, default_value = "build")]
    out_dir: PathBuf,
    #[arg(long, help = "Generate AI-Chat prompts for missing/invalid assets")]
    generate_prompts: bool,
}

#[derive(Debug, Deserialize)]
struct GridFile {
    map_id: String,
    grid_size: GridSize,
    cells: Vec<GridCell>,
}
#[derive(Debug, Deserialize)]
struct GridSize { cols: u32, rows: u32 }
#[derive(Debug, Deserialize)]
struct GridCell {
    col: u32,
    row: u32,
    tile_type: String,
    #[serde(default)]
    role_tags: Vec<String>,
}

#[derive(Debug, Deserialize)]
struct TilesetFile {
    engine: String,
    tile_mappings: Vec<TileMapping>,
}
#[derive(Debug, Deserialize)]
struct TileMapping {
    tile_type: String,
    asset_id: String,
}

#[derive(Debug, Serialize)]
struct AIAssetPrompt {
    map_id: String,
    missing_tile_type: String,
    target_engine: String,
    n64_constraints: String,
    prompt_text: String,
    validation_rules: Vec<String>,
}

fn main() -> Result<()> {
    let cli = Cli::parse();
    let grid_text = fs::read_to_string(&cli.grid_path)
        .with_context(|| format!("Failed to read grid: {:?}", cli.grid_path))?;
    let grid: GridFile = serde_json::from_str(&grid_text)
        .with_context(|| "Invalid grid JSON")?;
    let tileset_text = fs::read_to_string(&cli.tileset_path)
        .with_context(|| format!("Failed to read tileset: {:?}", cli.tileset_path))?;
    let tileset: TilesetFile = serde_json::from_str(&tileset_text)
        .with_context(|| "Invalid tileset JSON")?;

    let tileset_types: HashSet<&str> = tileset.tile_mappings.iter()
        .map(|m| m.tile_type.as_str()).collect();
    let mut missing: BTreeMap<String, Vec<String>> = BTreeMap::new();

    for cell in &grid.cells {
        if !tileset_types.contains(cell.tile_type.as_str()) {
            missing.entry(cell.tile_type.clone()).or_default().push(cell.role_tags.join(", "));
        }
    }

    if cli.generate_prompts && !missing.is_empty() {
        let prompts = generate_ai_prompts(&grid.map_id, &missing, &tileset.engine);
        let out_path = cli.out_dir.join("ai_prompts.json");
        fs::write(&out_path, serde_json::to_string_pretty(&prompts)?)
            .with_context(|| "Failed to write AI prompts")?;
        println!("[AI] Generated {} tileset prompts → {:?}", prompts.len(), out_path);
    }

    println!("[VALIDATE] Missing tile_types: {}", missing.len());
    if !missing.is_empty() {
        for (tile, tags) in &missing {
            println!("  • {} (tags: {})", tile, tags.join(", "));
        }
    }
    Ok(())
}

/// AI-Chat Helper: Formats structured context for LLM asset generation
pub fn format_llm_context(map_id: &str, missing: &BTreeMap<String, Vec<String>>, engine: &str) -> String {
    let mut ctx = format!("# Conker AI Asset Generation Context\n## Map: {}\n## Engine Target: {}\n\n", map_id, engine);
    ctx.push_str("### N64 Constraints\n- Grid cell: 4.0 world units\n- Pivot: floor-center, 0,0,0\n- Max poly budget per tile: ~800 tris\n- Texture: 256x256 max, 4-8 bit palette or compressed DXT1\n\n### Missing Tile Types & Role Tags\n");
    for (tile, tags) in missing {
        ctx.push_str(&format!("- `{}` → Tags: {}\n", tile, tags.join(", ")));
    }
    ctx.push_str("\nGenerate a single JSON response matching `AIAssetPrompt` schema for each missing type.\n");
    ctx
}

/// AI-Chat Helper: Generates structured prompts for AI image/mesh generators
fn generate_ai_prompts(map_id: &str, missing: &BTreeMap<String, Vec<String>>, engine: &str) -> Vec<AIAssetPrompt> {
    missing.iter().map(|(tile_type, tags)| {
        let constraint = "4x4 N64 grid, low-poly, flat lighting, sharp UV seams, palette-friendly, pivot at floor center";
        let prompt = format!("Create a Conker: Live & Uncut multiplayer tile named '{}'. Style: N64-era industrial/alien base. Mesh must snap to 4x4 grid. Tags: {}", tile_type, tags.join(", "));
        AIAssetPrompt {
            map_id: map_id.to_string(),
            missing_tile_type: tile_type.clone(),
            target_engine: engine.to_string(),
            n64_constraints: constraint.to_string(),
            prompt_text: prompt,
            validation_rules: vec![
                "Vertex count <= 800",
                "Single 256x256 texture or UV atlas",
                "No floating vertices, manifold mesh",
                "Pivot at (0,0,0) on floor plane"
            ]
        }
    }).collect()
}
