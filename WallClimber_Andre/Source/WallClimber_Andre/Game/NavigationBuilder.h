#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "NavigationBuilder.generated.h"

USTRUCT(BlueprintType)
struct FNavigationNode
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "NavigationNode")
	FVector Location;

	UPROPERTY(VisibleAnywhere, Category = "NavigationNode")
	FIntPoint ID;

	UPROPERTY(VisibleAnywhere, Category = "NavigationNode")
	bool bIsValid;

	// Equality operator - Must be included to work with Algo::FindBy
	bool operator==(const FNavigationNode& Other) const
	{
		return ID == Other.ID;
	}
};

UCLASS()
class WALLCLIMBER_ANDRE_API ANavigationBuilder : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ANavigationBuilder();

	UPROPERTY(EditAnywhere, Category = "Default")
	bool bEnableDebugging = false;

	UPROPERTY(BlueprintReadOnly, Category = "Default")
	UBoxComponent* NavMeshBoundaries;

	UPROPERTY(EditAnywhere, Category = "Default")
	FVector NavMeshExtents = FVector(1000, 1000, 200);

	UPROPERTY(EditAnywhere, Category = "Default")
	float NavMeshDensity = 10.f; // Suggested number of vectors per 1000 unreal units

	UPROPERTY(EditAnywhere, Category = "Default")
	int32 ThresholdBuffer = 2;

	UPROPERTY(EditAnywhere, Category = "Default")
	float SpacingUnits = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Default")
	bool bNavigationActive = false; // Default to false, will be set true when navigation is built

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:
	UFUNCTION(BlueprintCallable, CallInEditor)
	void BuildNavigation();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void ClearDebugObjects();

	TArray<FNavigationNode> GetNavigationNodesArray();
	TArray<FIntPoint> GetCircularNeighbors(int32 Radius);

private:
	TArray<FVector> NavigationGrid;
	TArray<FIntPoint> IDArray;
	TArray<FNavigationNode> NavigationNodesArray;

	void InitializeNavigationGrid();
	void ConstructNavigationNodes();
	void ApplyThresholdBuffer();
	void CreateDebugGrid();
	
	TArray<FNavigationNode> FindThresholdNodes(TArray<FNavigationNode> SourceNodes);
};
