# Filename: CKT_ModeProfileFilter.gd
# Destination: res://Conkart/Systems/CKT_ModeProfileFilter.gd
class_name CKT_ModeProfileFilter
extends Node

# AI‑Chat Helper: Generates a prompt for LLMs to create a new mode profile entry
static func generate_ai_mode_config_prompt(map_id: String, available_zones: PackedStringArray) -> String:
	return """# Conkart Mode Configuration Prompt
## Map: %s
## Available Zones: %s

Generate a valid ModeProfileEntry JSON object for a new game mode.
Constraints:
- ModeId must be PascalCase, no spaces.
- LightingProfile must be one of: BeachDay, BeachNightStealth, WarOvercast, InteriorDim.
- Only include zones/role_tags that exist in the map's grid/entities.
Output format: { "ModeId": "...", "DisplayName": "...", ... }""" % [map_id, ", ".join(available_zones)]

# AI‑Chat Helper: Minimal runtime validation; full validation should use schemaguard in CI
static func validate_mode_profile_against_schema(json_content: String) -> Dictionary:
	var result = { "valid": false, "error": "" }
	var parsed = JSON.parse_string(json_content)
	if typeof(parsed) != TYPE_DICTIONARY:
		result.error = "Failed to parse JSON"
		return result
	if not parsed.has("version") or not parsed.has("mapid") or not parsed.has("modes"):
		result.error = "Missing required top‑level fields"
		return result
	result.valid = true
	return result

static func is_role_tag_enabled(enabled_tags: PackedStringArray, entity_tags: PackedStringArray) -> bool:
	for tag in entity_tags:
		if enabled_tags.has(tag):
			return true
	return false

static func load_mode_profile(profile_path: String, mode_id: String) -> Dictionary:
	var file = FileAccess.open(profile_path, FileAccess.READ)
	if not file:
		return {}
	var content = file.get_as_text()
	file.close()
	var parsed = JSON.parse_string(content)
	if typeof(parsed) != TYPE_DICTIONARY or not parsed.has("modes"):
		return {}
	for mode in parsed["modes"]:
		if mode.get("id") == mode_id:
			return {
				"ModeId": mode.get("id"),
				"DisplayName": mode.get("displayname"),
				"EnabledZones": mode.get("enabledzones", []),
				"EnabledSpawnRoleTags": mode.get("enabledspawnroletags", []),
				"EnabledObjectiveRoleTags": mode.get("enabledobjectiveroletags", []),
				"EnabledHazardRoleTags": mode.get("enabledhazardroletags", []),
				"LightingProfile": mode.get("lightingprofile")
			}
	return {}

static func filter_spawn_ids_by_mode(all_spawn_ids: PackedStringArray, profile: Dictionary, spawn_id_to_role_tags: Dictionary) -> PackedStringArray:
	var filtered: PackedStringArray = []
	for id in all_spawn_ids:
		if spawn_id_to_role_tags.has(id):
			var tags: PackedStringArray = spawn_id_to_role_tags[id]
			if is_role_tag_enabled(profile["EnabledSpawnRoleTags"], tags):
				filtered.append(id)
	return filtered
