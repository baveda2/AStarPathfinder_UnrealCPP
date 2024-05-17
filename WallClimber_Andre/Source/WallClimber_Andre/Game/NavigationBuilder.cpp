#include "NavigationBuilder.h"
#include "Algo/Find.h"

// Sets default values
ANavigationBuilder::ANavigationBuilder()
{
	// Set this actor to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Setup NavMesh
	NavMeshBoundaries = CreateDefaultSubobject<UBoxComponent>(TEXT("NavMeshBoundaries"));
	NavMeshBoundaries->SetupAttachment(RootComponent);
	NavMeshBoundaries->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANavigationBuilder::OnConstruction(const FTransform& Transform)
{
	NavMeshBoundaries->SetBoxExtent(NavMeshExtents);
}

void ANavigationBuilder::BeginPlay()
{
	Super::BeginPlay();

	BuildNavigation();

	if (!bNavigationActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("Navigation not built! Make sure the navigation is active."));
	}

	if (bEnableDebugging)
	{
		UE_LOG(LogTemp, Warning, TEXT("DEBUG GRID BUILT"));
		CreateDebugGrid();
	}
}

// Editor-callable function to build the navigation nodes
void ANavigationBuilder::BuildNavigation()
{
	InitializeNavigationGrid();
	ConstructNavigationNodes();
	ApplyThresholdBuffer();
	CreateDebugGrid();

	// Visualize in editor to check if the Navigation Grid is Active
	bNavigationActive = NavigationNodesArray.Num() > 0;
}

// Build a base grid with provided extents, density and spacing
// Those variables makes a precise control of the number of nodes
void ANavigationBuilder::InitializeNavigationGrid()
{
	// Cleanup
	NavigationGrid.Empty();
	IDArray.Empty();
	NavigationNodesArray.Empty();

	// Create grid based on Box Extents and density
	float Spacing = SpacingUnits / NavMeshDensity;
	int32 TotalPointsX = FMath::FloorToInt(NavMeshExtents.X * 2 / Spacing);
	int32 TotalPointsY = FMath::FloorToInt(NavMeshExtents.Y * 2 / Spacing);

	// Generate base grid
	for (int32 i = 1; i < TotalPointsX; i++)
	{
		for (int32 j = 1; j < TotalPointsY; j++)
		{
			float X = (i * Spacing) - NavMeshExtents.X;
			float Y = (j * Spacing) - NavMeshExtents.Y;
			FVector MeshPoint = FVector(X, Y, NavMeshExtents.Z);
			NavigationGrid.Add(MeshPoint);
			IDArray.Add(FIntPoint(i, j));
		}
	}
}

// Use the default grid to create the navigation nodes array
void ANavigationBuilder::ConstructNavigationNodes()
{
	for (int32 i = 0; i < NavigationGrid.Num(); ++i)
	{
		const FVector& MeshPoint = NavigationGrid[i];
		FVector StartPoint = GetActorTransform().TransformPosition(MeshPoint);
		FVector EndPoint = GetActorTransform().TransformPosition(MeshPoint - FVector(0, 0, NavMeshExtents.Z * 2));

		FHitResult GridHitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);

		bool bHit = GetWorld()->LineTraceSingleByChannel(GridHitResult, StartPoint, EndPoint, ECC_Visibility, CollisionParams);

		if (bHit)
		{
			FNavigationNode CurrentNode;
			CurrentNode.Location = GridHitResult.Location;
			CurrentNode.bIsValid = GridHitResult.GetActor()->Tags.Contains(FName("Walkable"));
			CurrentNode.ID = IDArray[i];
			NavigationNodesArray.Add(CurrentNode);
		}
	}
	//UE_LOG(LogTemp, Warning, TEXT("Constructed %d navigation nodes."), NavigationNodesArray.Num());
}

// Expand the threshold of invalid nodes (obstacles). The main purpose is to avoid navigation too close to obstacles, preventing clipping
void ANavigationBuilder::ApplyThresholdBuffer()
{
	TArray<FNavigationNode> CulledGrid = NavigationNodesArray; // Nodes to modify
	TArray<FNavigationNode> ThresholdNodes = FindThresholdNodes(NavigationNodesArray); // Initial threshold nodes

	TArray<FIntPoint> CircularNeighbors = GetCircularNeighbors(ThresholdBuffer); // Get neighbors within the circular range

	for (const FNavigationNode& Node : ThresholdNodes)
	{
		for (const FIntPoint& NeighborOffset : CircularNeighbors)
		{
			FIntPoint TargetID = Node.ID + NeighborOffset;
			FNavigationNode* FoundNode = Algo::FindBy(CulledGrid, TargetID, &FNavigationNode::ID);

			if (FoundNode != nullptr && FoundNode->bIsValid)
			{
				int32 FoundNodeIndex = CulledGrid.IndexOfByKey(*FoundNode);
				if (FoundNodeIndex != INDEX_NONE)
				{
					CulledGrid[FoundNodeIndex].bIsValid = false; // Invalidate the node
				}
			}
		}
	}

	NavigationNodesArray = CulledGrid; // Update the main navigation nodes array
}


// Show debug grid with valid and not valid nodes
void ANavigationBuilder::CreateDebugGrid()
{
	if (bEnableDebugging)
	{
		FlushPersistentDebugLines(GetWorld());
		for (const FNavigationNode& Node : NavigationNodesArray)
		{
			FColor NodeColor = Node.bIsValid ? FColor::Green : FColor::Red;
			DrawDebugPoint(GetWorld(), Node.Location, 10.f, NodeColor, true, -1.f);
		}
	}
}

// Editor-callable function to clear debug objects
void ANavigationBuilder::ClearDebugObjects()
{
	FlushPersistentDebugLines(GetWorld());
}


// Helper function to find nodes inside ThresholdBuffer range
TArray<FIntPoint> ANavigationBuilder::GetCircularNeighbors(int32 Radius)
{
	TArray<FIntPoint> Neighbors;

	for (int32 X = -Radius; X <= Radius; ++X)
	{
		for (int32 Y = -Radius; Y <= Radius; ++Y)
		{
			if (X == 0 && Y == 0)
				continue;

			if (X * X + Y * Y <= Radius * Radius)
				Neighbors.Add(FIntPoint(X, Y));
		}
	}

	return Neighbors;
}

// Helper function to find threshold nodes
TArray<FNavigationNode> ANavigationBuilder::FindThresholdNodes(TArray<FNavigationNode> SourceNodes)
{
	TArray<FNavigationNode> ThresholdNodes;
	TArray<FIntPoint> ImmediateNeighbors = GetCircularNeighbors(1);

	for (const FNavigationNode& Node : SourceNodes)
	{
		if (!Node.bIsValid)
		{
			for (const FIntPoint& Neighbor : ImmediateNeighbors)
			{
				FIntPoint TargetID = Node.ID + Neighbor;
				FNavigationNode* FoundNode = Algo::FindBy(SourceNodes, TargetID, &FNavigationNode::ID);

				if (FoundNode != nullptr && FoundNode->bIsValid)
				{
					ThresholdNodes.Add(Node);
					break;
				}
			}
		}
	}
	return ThresholdNodes;
}

// Public method to return NavigationNodes grid - Core variables to build A* Algorithm
TArray<FNavigationNode> ANavigationBuilder::GetNavigationNodesArray()
{
	return NavigationNodesArray;
}
