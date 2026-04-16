conkart-ai-codegen/
├── .github/workflows/        # CI/CD for asset generation (optional)
├── src/
│   ├── main.rs               # CLI entry point, parses commands and routes tasks
│   ├── ai/
│   │   ├── mod.rs            # AI model interface and prompt management
│   │   └── prompts.rs        # System prompts for different asset types
│   ├── generator/
│   │   ├── mod.rs            # Asset generation logic (procedural + AI)
│   │   ├── tileset.rs        # Logic for generating tilesets
│   │   ├── sprite.rs         # Logic for generating sprites
│   │   └── wfc.rs            # Wave Function Collapse algorithm
│   ├── export/
│   │   ├── mod.rs            # Export modules for different formats
│   │   ├── n64.rs            # Export logic for N64-compatible formats
│   │   ├── godot.rs          # Export logic for Godot (.tres, .tscn)
│   │   └── unity.rs          # Export logic for Unity (.asset, .meta)
│   └── utils/
│       ├── mod.rs            # Common helper functions
│       ├── fs.rs             # Safe filesystem operations
│       └── validation.rs     # Validation of generated assets
├── knowledge_graph/           # JSON/YAML files describing asset relationships
│   ├── tileset_graph.json    # Describes tile adjacency rules, palettes
│   └── asset_graph.json      # Describes high-level asset dependencies
├── templates/                 # Base templates for generated files
│   ├── tileset_template.txt
│   └── sprite_template.txt
├── Cargo.toml
└── README.md
