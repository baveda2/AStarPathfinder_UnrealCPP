/*
    PathfindingComponent.cpp
    Purpose: Implementation of the PathfindingComponent class. This class handles the initialization of pathfinding nodes and provides functionality to find paths using the A* algorithm.
    It interacts with the NavigationBuilder to retrieve the navigation grid and processes pathfinding requests.
*/

#include "PathfindingComponent.h"
#include "EngineUtils.h"

// Constructor
UPathfindingComponent::UPathfindingComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UPathfindingComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializePathfinding();
}

// Initializes pathfinding nodes from the NavigationBuilder
void UPathfindingComponent::InitializePathfinding()
{
    // Find the NavigationBuilder in the world and initialize pathfinding nodes
    for (TActorIterator<ANavigationBuilder> It(GetWorld()); It; ++It)
    {
        NavBuilder = *It;
        if (NavBuilder)
        {
            UE_LOG(LogTemp, Warning, TEXT("NavBuilder found"));
            const auto& NavNodes = NavBuilder->GetNavigationNodesArray();
            UE_LOG(LogTemp, Warning, TEXT("NavBuilder has %d nodes"), NavNodes.Num());

            // Convert FNavigationNode to FPathfindingNode and store them
            for (const FNavigationNode& Node : NavNodes)
            {
                FPathfindingNode NewNode;
                NewNode.Location = Node.Location;
                NewNode.ID = Node.ID;
                NewNode.bIsValid = Node.bIsValid;
                PathfindingNodes.Add(NewNode);
                //UE_LOG(LogTemp, Warning, TEXT("Added Node ID: %s, Location: %s"), *NewNode.ID.ToString(), *NewNode.Location.ToString());
            }
            break;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("PathfindingNodes initialized with %d nodes"), PathfindingNodes.Num());
}


// Finds a path between two locations using the A* algorithm
TArray<FPathfindingNode> UPathfindingComponent::FindPath(const FVector& StartLocation, const FVector& EndLocation)
{
    FPathfindingNode* StartNode = GetClosestNode(StartLocation);
    FPathfindingNode* EndNode = GetClosestNode(EndLocation);

    if (StartNode && EndNode)
    {
        return CalculateAStarPath(StartNode, EndNode);
    }

    return TArray<FPathfindingNode>();
}

// Finds the closest pathfinding node to a given location
FPathfindingNode* UPathfindingComponent::GetClosestNode(const FVector& Location)
{
    FPathfindingNode* ClosestNode = nullptr;
    float ClosestDistanceSquared = FLT_MAX;

    //UE_LOG(LogTemp, Warning, TEXT("PathfindingNodes Count: %d"), PathfindingNodes.Num());

    for (FPathfindingNode& Node : PathfindingNodes)
    {
        if (Node.bIsValid)
        {
            float DistanceSquared = FVector::DistSquared(Location, Node.Location);
            //UE_LOG(LogTemp, Warning, TEXT("Checking Node at %s, DistanceSquared: %f"), *Node.Location.ToString(), DistanceSquared);

            if (DistanceSquared < ClosestDistanceSquared)
            {
                ClosestNode = &Node;
                ClosestDistanceSquared = DistanceSquared;
            }
        }
    }

    if (ClosestNode)
    {
        //UE_LOG(LogTemp, Warning, TEXT("Closest Node Found at %s"), *ClosestNode->Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No Closest Node Found"));
    }

    return ClosestNode;
}


// Calculates the shortest path between two nodes using the A* algorithm
TArray<FPathfindingNode> UPathfindingComponent::CalculateAStarPath(FPathfindingNode* StartNode, FPathfindingNode* EndNode)
{
    // The open set, nodes to be evaluated
    TArray<FPathfindingNode*> OpenSet;
    // The closed set, nodes already evaluated
    TSet<FPathfindingNode*> ClosedSet;

    // Add the start node to the open set
    OpenSet.Add(StartNode);

    // While the open set is not empty
    while (OpenSet.Num() > 0)
    {
        // Find the node in the open set with the lowest F score
        FPathfindingNode* CurrentNode = OpenSet[0];
        for (int32 i = 1; i < OpenSet.Num(); ++i)
        {
            if (OpenSet[i]->FCost < CurrentNode->FCost)
            {
                CurrentNode = OpenSet[i];
            }
        }

        // If the current node is the end node, reconstruct the path and return it
        if (CurrentNode == EndNode)
        {
            TArray<FPathfindingNode> Path;
            while (CurrentNode != nullptr)
            {
                Path.Add(*CurrentNode);
                CurrentNode = (CurrentNode->ParentIndex != INDEX_NONE) ? &PathfindingNodes[CurrentNode->ParentIndex] : nullptr;
            }
            Algo::Reverse(Path);
            return Path;
        }

        // Move the current node from the open set to the closed set
        OpenSet.Remove(CurrentNode);
        ClosedSet.Add(CurrentNode);

        // For each neighbor of the current node
        for (const FIntPoint NeighborOffset : NavBuilder->GetCircularNeighbors(1))
        {
            FIntPoint NeighborID = CurrentNode->ID + NeighborOffset;

            FPathfindingNode* NeighborNode = nullptr;
            for (FPathfindingNode& Node : PathfindingNodes)
            {
                if (Node.ID == NeighborID && Node.bIsValid)
                {
                    NeighborNode = &Node;
                    break;
                }
            }



            if (NeighborNode && !ClosedSet.Contains(NeighborNode))
            {
                // The distance from start to the neighbor
                int32 MovementCost = (FMath::Abs(NeighborOffset.X) + FMath::Abs(NeighborOffset.Y) == 1) ? 10 : 14; // Cross shape cost is 10, diagonal (X shape) cost is 14
                int32 TentativeGScore = CurrentNode->GCost + MovementCost;

                if (!OpenSet.Contains(NeighborNode))
                {
                    OpenSet.Add(NeighborNode);
                }
                else if (TentativeGScore >= NeighborNode->GCost)
                {
                    continue; // This is not a better path
                }

                // This path is the best so far, record it
                NeighborNode->ParentIndex = PathfindingNodes.IndexOfByKey(*CurrentNode);
                NeighborNode->GCost = TentativeGScore;
                NeighborNode->HCost = FMath::Abs(NeighborID.X - EndNode->ID.X) + FMath::Abs(NeighborID.Y - EndNode->ID.Y); // Manhattan distance as heuristic
                NeighborNode->FCost = NeighborNode->GCost + NeighborNode->HCost; // Update FCost
            }
        }
    }

    // If we get here, there was no path found
    return TArray<FPathfindingNode>();
}
