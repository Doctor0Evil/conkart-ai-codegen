-- Filename: ai_tileset_bridge.lua
-- Destination: scripts/conkart/ai_tileset_bridge.lua
-- N64-Era AI Asset Generator Bridge for Conker: Live & Uncut

local AI = {}

AI.N64_CONSTRAINTS = {
    max_vertices = 800,
    max_texture_dim = 256,
    cell_size = 4.0,
    pivot_floor_center = true,
    allowed_formats = {"png", "tga"},
    color_depth = "8bit"
}

-- AI-Chat Helper: Formats a prompt optimized for LLM/SD mesh & texture generation
function AI.format_n64_texture_prompt(tile_type, role_tags, theme)
    local tags = type(role_tags) == "table" and table.concat(role_tags, ", ") or tostring(role_tags)
    local prompt = string.format(
        "Generate N64-style game asset for '%s'. Theme: '%s'. Tags: %s. " ..
        "Style: Conker: Live & Uncut revival. Low-poly (<=800 tris), 256x256 texture, " ..
        "sharp UV edges, flat lighting, pivot at floor center (0,0,0). " ..
        "Output: JSON with 'mesh_hint', 'uv_layout', 'palette_swatches', 'validation_notes'.",
        tile_type, theme, tags
    )
    return prompt
end

-- AI-Chat Helper: Validates AI-generated asset metadata against N64 invariants
function AI.validate_ai_output_n64_compliant(ai_response)
    if not ai_response.mesh_hint or not ai_response.uv_layout then
        return false, "Missing required mesh/UV data"
    end
    if ai_response.tri_count and ai_response.tri_count > AI.N64_CONSTRAINTS.max_vertices then
        return false, "Triangle count exceeds N64 budget"
    end
    if ai_response.texture_size and (ai_response.texture_size.w > AI.N64_CONSTRAINTS.max_texture_dim or 
       ai_response.texture_size.h > AI.N64_CONSTRAINTS.max_texture_dim) then
        return false, "Texture exceeds 256x256 limit"
    end
    return true, "Valid"
end

-- AI-Chat Helper: Generates a manifest-ready JSON string for AI pipeline ingestion
function AI.generate_llm_tileset_manifest(tile_entries)
    local manifest = {
        version = "1.0.0",
        project = "Conker_Live_Uncut",
        n64_rules = AI.N64_CONSTRAINTS,
        assets = {}
    }
    for _, entry in ipairs(tile_entries) do
        table.insert(manifest.assets, {
            id = entry.id,
            prompt = AI.format_n64_texture_prompt(entry.id, entry.tags, entry.theme),
            validation_status = "pending",
            target_engine = "unreal/unity/godot"
        })
    end
    -- Simple JSON serialization (replace with proper lib in production)
    local json = "{\n"
    json = json .. string.format('  "version": "%s",\n', manifest.version)
    json = json .. string.format('  "project": "%s",\n', manifest.project)
    json = json .. '  "assets": [\n'
    for i, a in ipairs(manifest.assets) do
        json = json .. string.format('    {"id": "%s", "prompt": "%s"}%s\n', a.id, a.prompt, i < #manifest.assets and "," or "")
    end
    json = json .. '  ]\n}'
    return json
end

return AI
