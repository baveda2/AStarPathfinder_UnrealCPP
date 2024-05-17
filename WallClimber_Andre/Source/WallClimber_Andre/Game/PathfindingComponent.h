/*
    PathfindingComponent.h
    Purpose: Header file for the PathfindingComponent class, which is responsible for finding paths in a navigation grid using the A* algorithm.
    The component finds the NavigationBuilder in the world, initializes pathfinding nodes, and provides a method to find a path between two points.
*/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationBuilder.h"
#include "PathfindingComponent.generated.h"

// Structure representing a node in the pathfinding grid
USTRUCT(BlueprintType)
struct FPathfindingNode
{
    GENERATED_BODY()

    // World location of the node
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    FVector Location;

    // Unique identifier for the node
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    FIntPoint ID;

    // Cost from the starting node to this node
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    int32 GCost;

    // Heuristic estimate from this node to the end node
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    int32 HCost;

    // Sum of GCost and HCost
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    int32 FCost;

    // Index of the parent node in the pathfinding array
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    int32 ParentIndex;

    // Whether the node is traversable
    UPROPERTY(VisibleAnywhere, Category = "PathfindingNode")
    bool bIsValid;

    // Default constructor
    FPathfindingNode() : Location(FVector::ZeroVector), ID(FIntPoint(-1, -1)), GCost(0), HCost(0), FCost(0), ParentIndex(INDEX_NONE), bIsValid(true) {}

    // Equality operator for node comparison
    bool operator==(const FPathfindingNode& Other) const
    {
        return ID == Other.ID;
    }
};

// Component class for pathfinding
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class WALLCLIMBER_ANDRE_API UPathfindingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    // Constructor
    UPathfindingComponent();

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

public:
    // Initializes pathfinding nodes from the NavigationBuilder
    void InitializePathfinding();

    // Finds a path between two locations using the A* algorithm
    TArray<FPathfindingNode> FindPath(const FVector& StartLocation, const FVector& EndLocation);

    // Finds the closest pathfinding node to a given location
    FPathfindingNode* GetClosestNode(const FVector& Location);

    // Calculates the shortest path between two nodes using the A* algorithm
    TArray<FPathfindingNode> CalculateAStarPath(FPathfindingNode* StartNode, FPathfindingNode* EndNode);

private:

    ANavigationBuilder* NavBuilder;

    // Array of pathfinding nodes used by the A* algorithm
    TArray<FPathfindingNode> PathfindingNodes;


};
