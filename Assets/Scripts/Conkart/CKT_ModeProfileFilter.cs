// Filename: CKT_ModeProfileFilter.cs
// Destination: Assets/Scripts/Conkart/CKT_ModeProfileFilter.cs
using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;

namespace Conkart.Multiplayer
{
    [System.Serializable]
    public class ModeProfileEntry
    {
        public string ModeId;
        public string DisplayName;
        public List<string> EnabledZones;
        public List<string> EnabledSpawnRoleTags;
        public List<string> EnabledObjectiveRoleTags;
        public List<string> EnabledHazardRoleTags;
        public string LightingProfile; // enum key
    }

    [System.Serializable]
    public class ModeProfileFile
    {
        public string version;
        public string mapid;
        public string schemaversion;
        public List<ModeProfileEntry> modes;
    }

    public static class CKT_ModeProfileFilter
    {
        // AI‑Chat Helper: Generates a prompt for LLMs to create a new mode profile entry
        public static string GenerateAI_ModeConfigPrompt(string mapId, List<string> availableZones)
        {
            return $@"# Conkart Mode Configuration Prompt
## Map: {mapId}
## Available Zones: {string.Join(", ", availableZones)}

Generate a valid ModeProfileEntry JSON object for a new game mode.
Constraints:
- ModeId must be PascalCase, no spaces.
- LightingProfile must be one of: BeachDay, BeachNightStealth, WarOvercast, InteriorDim.
- Only include zones/role_tags that exist in the map's grid/entities.
Output format: {{ ""ModeId"": ""..."", ""DisplayName"": ""..."", ... }}";
        }

        // AI‑Chat Helper: Validates minimal structure; full validation should use schemaguard in CI
        public static bool ValidateModeProfileAgainstSchema(string jsonContent, out string error)
        {
            error = string.Empty;
            try
            {
                var root = JsonUtility.FromJson<ModeProfileFile>(jsonContent);
                if (string.IsNullOrEmpty(root.version) || string.IsNullOrEmpty(root.mapid) || root.modes == null || root.modes.Count == 0)
                {
                    error = "Missing required top‑level fields or empty modes array";
                    return false;
                }
                return true;
            }
            catch (Exception ex)
            {
                error = $"JSON parse error: {ex.Message}";
                return false;
            }
        }

        public static bool IsRoleTagEnabled(List<string> enabledTags, List<string> entityTags)
        {
            return entityTags.Any(tag => enabledTags.Contains(tag));
        }

        public static ModeProfileEntry LoadModeProfile(string profileJson, string modeId)
        {
            var file = JsonUtility.FromJson<ModeProfileFile>(profileJson);
            return file?.modes?.FirstOrDefault(m => m.ModeId == modeId);
        }

        public static List<string> FilterSpawnIdsByMode(List<string> allSpawnIds, ModeProfileEntry profile, Dictionary<string, List<string>> spawnIdToRoleTags)
        {
            var filtered = new List<string>();
            foreach (var id in allSpawnIds)
            {
                if (spawnIdToRoleTags.TryGetValue(id, out var tags) && IsRoleTagEnabled(profile.EnabledSpawnRoleTags, tags))
                {
                    filtered.Add(id);
                }
            }
            return filtered;
        }
    }
}
