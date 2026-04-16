// Filename: CKT_ModeProfileFilter.h
// Destination: Source/Conkart/Public/CKT_ModeProfileFilter.h
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CKT_ModeProfileFilter.generated.h"

USTRUCT(BlueprintType)
struct FCKTModeProfileEntry
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    FString ModeId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    FString DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    TArray<FString> EnabledZones;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    TArray<FString> EnabledSpawnRoleTags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    TArray<FString> EnabledObjectiveRoleTags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    TArray<FString> EnabledHazardRoleTags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Mode")
    FName LightingProfile;
};

UCLASS(BlueprintType)
class UCKT_ModeProfileFilter : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Conkart|AI")
    static FString GenerateAI_ModeConfigPrompt(const FString& MapId, const TArray<FString>& AvailableZones);

    UFUNCTION(BlueprintCallable, Category="Conkart|AI")
    static bool ValidateModeProfileAgainstSchema(const FString& JsonContent, FString& OutError);

    UFUNCTION(BlueprintPure, Category="Conkart|Mode")
    static bool IsRoleTagEnabled(const TArray<FString>& EnabledTags, const TArray<FString>& EntityTags);

    UFUNCTION(BlueprintCallable, Category="Conkart|Mode")
    static FCKTModeProfileEntry LoadModeProfile(const FString& ProfilePath, const FString& ModeId);

    UFUNCTION(BlueprintPure, Category="Conkart|Mode")
    static TArray<FString> FilterSpawnIdsByMode(const TArray<FString>& AllSpawnIds, const FCKTModeProfileEntry& Profile, const TMap<FString, TArray<FString>>& SpawnIdToRoleTags);
};
