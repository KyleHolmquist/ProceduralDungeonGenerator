// Fill out your copyright notice in the Description page of Project Settings.


#include "DungeonRoom.h"
#include "GridSpace.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

// Sets default values
ADungeonRoom::ADungeonRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>("Root");
	SetRootComponent(Root);

}

void ADungeonRoom::Initialize(int32 InNumTiles)
{
	NumTiles = InNumTiles;
}

// Called when the game starts or when spawned
void ADungeonRoom::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADungeonRoom::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!FoundEdges)
	{
		FindEdges();
	}

}

void ADungeonRoom::AddTile(AGridSpace* Tile)
{
	if (!Tile) return;

	Tiles.Add(Tile);

	//Group in the world for easy transform/visibility
	Tile->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);

	Tile->SetOwner(this);
}

FVector ADungeonRoom::GetCenter() const
{
	if (Tiles.Num() == 0) return FVector::ZeroVector;

	FVector Sum = FVector::ZeroVector;
	for (const AGridSpace* T : Tiles)
	{
		if (T) Sum += T->GetActorLocation();
	}
	return Sum / FMath::Max(1, Tiles.Num());
}

void ADungeonRoom::FindEdges()
{
	TArray<TObjectPtr<AGridSpace>> EdgeSpaces;

	for (TObjectPtr<AGridSpace> T : Tiles)
	{
		if (!T) continue;
		//Get an array of empty spaces surrounding the tile
		const TArray<FVector> EmptyNeighbors = T->GetEmptyNeighbors();
		//Check to see if the space is an edge space (edges likely to have 2-3 empty neighbors)
		if (EmptyNeighbors.Num() > 0 && EmptyNeighbors.Num() < 4)
		{
			//Add to new array of edge pieces
			EdgeSpaces.Add(T);
			//Get the Grid Space's mesh
			if (UStaticMeshComponent* Mesh = T->FindComponentByClass<UStaticMeshComponent>())
			{
				//Apply a color to the piece for debug
				if (EdgeMaterial) Mesh->SetMaterial(0, EdgeMaterial);
			}

		}
	}
	if (EdgeSpaces.Num() > 1)
	{
		FoundEdges = true;
	}
	if (EdgeSpaces.Num() < 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("No edges "), *GetName());
		FoundEdges = false;
	}
}