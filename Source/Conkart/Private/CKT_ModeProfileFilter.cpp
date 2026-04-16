// Filename: CKT_ModeProfileFilter.cpp
// Destination: Source/Conkart/Private/CKT_ModeProfileFilter.cpp
#include "CKT_ModeProfileFilter.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Misc/FileHelper.h"

FString UCKT_ModeProfileFilter::GenerateAI_ModeConfigPrompt(const FString& MapId, const TArray<FString>& AvailableZones)
{
    FString Prompt = FString::Printf(TEXT(
        "# Conkart Mode Configuration Prompt\n"
        "## Map: %s\n"
        "## Available Zones: %s\n\n"
        "Generate a valid FCKTModeProfileEntry JSON object for a new game mode.\n"
        "Constraints:\n"
        "- ModeId must be PascalCase, no spaces.\n"
        "- LightingProfile must be one of: BeachDay, BeachNightStealth, WarOvercast, InteriorDim.\n"
        "- Only include zones/role_tags that exist in the map's grid/entities.\n"
        "Output format: { \"ModeId\": \"...\", \"DisplayName\": \"...\", ... }"
    ), *MapId, *FString::Join(AvailableZones, TEXT(", ")));
    return Prompt;
}

bool UCKT_ModeProfileFilter::ValidateModeProfileAgainstSchema(const FString& JsonContent, FString& OutError)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonContent);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OutError = TEXT("Failed to parse JSON");
        return false;
    }
    // Minimal runtime checks; full validation should use schemaguard in CI.
    if (!JsonObject->HasField(TEXT("version")) || !JsonObject->HasField(TEXT("mapid")) || !JsonObject->HasField(TEXT("modes")))
    {
        OutError = TEXT("Missing required top‑level fields");
        return false;
    }
    return true;
}

bool UCKT_ModeProfileFilter::IsRoleTagEnabled(const TArray<FString>& EnabledTags, const TArray<FString>& EntityTags)
{
    for (const FString& Tag : EntityTags)
    {
        if (EnabledTags.Contains(Tag)) { return true; }
    }
    return false;
}

FCKTModeProfileEntry UCKT_ModeProfileFilter::LoadModeProfile(const FString& ProfilePath, const FString& ModeId)
{
    FCKTModeProfileEntry Result;
    FString Content;
    if (!FFileHelper::LoadFileToString(Content, *ProfilePath)) { return Result; }
    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Content);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid()) { return Result; }
    const TArray<TSharedPtr<FJsonValue>>* ModesArray;
    if (!Root->TryGetArrayField(TEXT("modes"), ModesArray)) { return Result; }
    for (const TSharedPtr<FJsonValue>& ModeVal : *ModesArray)
    {
        const TSharedPtr<FJsonObject>* ModeObj = ModeVal->AsObject();
        if (!ModeObj || !(*ModeObj)->HasTypedField<EJson::String>(TEXT("id"))) { continue; }
        if ((*ModeObj)->GetStringField(TEXT("id")) != ModeId) { continue; }
        Result.ModeId = ModeId;
        Result.DisplayName = (*ModeObj)->GetStringField(TEXT("displayname"));
        Result.LightingProfile = FName(*(*ModeObj)->GetStringField(TEXT("lightingprofile")));
        // Arrays...
        auto ReadStringArray = [ModeObj](const FString& Key) {
            TArray<FString> Arr;
            const TArray<TSharedPtr<FJsonValue>>* ArrVal;
            if ((*ModeObj)->TryGetArrayField(Key, ArrVal)) {
                for (const TSharedPtr<FJsonValue>& V : *ArrVal) { Arr.Add(V->AsString()); }
            }
            return Arr;
        };
        Result.EnabledZones = ReadStringArray(TEXT("enabledzones"));
        Result.EnabledSpawnRoleTags = ReadStringArray(TEXT("enabledspawnroletags"));
        Result.EnabledObjectiveRoleTags = ReadStringArray(TEXT("enabledobjectiveroletags"));
        Result.EnabledHazardRoleTags = ReadStringArray(TEXT("enabledhazardroletags"));
        break;
    }
    return Result;
}

TArray<FString> UCKT_ModeProfileFilter::FilterSpawnIdsByMode(const TArray<FString>& AllSpawnIds, const FCKTModeProfileEntry& Profile, const TMap<FString, TArray<FString>>& SpawnIdToRoleTags)
{
    TArray<FString> Filtered;
    for (const FString& Id : AllSpawnIds)
    {
        if (const TArray<FString>* Tags = SpawnIdToRoleTags.Find(Id))
        {
            if (IsRoleTagEnabled(Profile.EnabledSpawnRoleTags, *Tags)) { Filtered.Add(Id); }
        }
    }
    return Filtered;
}
