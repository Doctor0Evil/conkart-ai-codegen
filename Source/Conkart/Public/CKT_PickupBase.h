// Filename: CKT_PickupBase.h
// Destination: Source/Conkart/Public/CKT_PickupBase.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CKT_PickupBase.generated.h"

UENUM(BlueprintType)
enum class ECKTPickupState : uint8
{
    Available,
    Taken,
    Respawning
};

USTRUCT(BlueprintType)
struct FCKTPickupSpec
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Pickup")
    FName AssetId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Pickup")
    bool bIsHeavyCarry = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Pickup")
    float RespawnTime = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|Pickup")
    float InteractionRadius = 200.0f;
};

USTRUCT(BlueprintType)
struct FCKTAIMetadata
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|AI")
    FString AIGenerationPrompt;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|AI")
    TArray<FString> AIVerificationTags;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Conkart|AI")
    bool bAIValidated = false;
};

UCLASS(Abstract)
class AConkart_API ACKT_PickupBase : public AActor
{
    GENERATED_BODY()
public:
    ACKT_PickupBase();
    virtual void BeginPlay() override;
    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Hit);

    UFUNCTION(BlueprintCallable, Category="Conkart|AI")
    FString GetAI_GenerationContext() const;

    UFUNCTION(BlueprintCallable, Category="Conkart|AI")
    void UpdateAssetMetadataForLLM(const FString& NewPrompt, const TArray<FString>& Tags);

    UFUNCTION(BlueprintCallable, Category="Conkart|AI")
    bool ValidateAIGeneratedMesh(UStaticMesh* NewMesh) const;

    UFUNCTION(BlueprintNativeEvent, Category="Conkart|Pickup")
    void ApplyPickup(AActor* Picker, const FName& WeaponId, bool bHeavy);
    virtual void ApplyPickup_Implementation(AActor* Picker, const FName& WeaponId, bool bHeavy) PURE_VIRTUAL(ACKT_PickupBase::ApplyPickup_Implementation,);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Conkart|Pickup")
    FCKTPickupSpec Spec;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Conkart|AI")
    FCKTAIMetadata AI_Meta;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conkart|Core")
    class USphereComponent* InteractionSphere;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Conkart|Core")
    UStaticMeshComponent* PickupMesh;
    ECKTPickupState State;
};
