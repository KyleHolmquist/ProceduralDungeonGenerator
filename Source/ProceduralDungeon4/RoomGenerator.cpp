// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomGenerator.h"
#include "GridSpace.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "ProceduralDungeonGameMode.h"
#include "DungeonRoom.h"

// Sets default values
ARoomGenerator::ARoomGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

}

// Called when the game starts or when spawned
void ARoomGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (!GridTileToSpawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("GridTileToSpawn not assigned in %s"), *GetName());
        return;
    }
	
	ADungeonRoom* DungeonRoom1 = GenerateRoom(numTiles);
	//Spawn player after generation
	if (AProceduralDungeonGameMode* GM = GetWorld()->GetAuthGameMode<AProceduralDungeonGameMode>())
	{
        UE_LOG(LogTemp, Warning, TEXT("Starting Spawn Player logic"));
		//Set start location to 0,0,0
		const FVector StartLoc = FVector(0.0f, 0.0f, 20.0f);

		//Set random starting rotation
		const float RandomYaw = FMath::FRandRange(0.f, 360.f);
        const FRotator StartRot(0.f, RandomYaw, 0.f);

		//GM->SpawnPlayerAtTransform(FTransform(StartRot, StartLoc));

		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC)
		{
			APawn* PlayerPawn = PC->GetPawn();
			if (PlayerPawn)
			{
				// Pick where you want to place the player — e.g. first tile center
				FVector TargetLocation = FVector(0.f, 0.f, 100.f);
				if (GridSpacesInRoom.Num() > 0 && GridSpacesInRoom[0])
				{
					TargetLocation = GridSpacesInRoom[0]->GetActorLocation() + FVector(0.f, 0.f, 100.f);
				}

				// Move and rotate player
				PlayerPawn->SetActorLocation(TargetLocation);
				PlayerPawn->SetActorRotation(StartRot);

				UE_LOG(LogTemp, Log, TEXT("Moved player to %s and facing %f degrees yaw."),
					*TargetLocation.ToString(), RandomYaw);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("PlayerPawn not found!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController not found!"));
		}
	}
		
}

// Called every frame
void ARoomGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoomGenerator::SpawnTile(const FVector& spawnLocation)
{
	AGridSpace* GridTile = GetWorld()->SpawnActor<AGridSpace>(
		GridTileToSpawn,
		spawnLocation,
		SpawnRotation,
		SpawnParams
	);

	if (GridTile)
	{
		tilesSpawned++;
		GridSpacesInRoom.Add(GridTile);
	}
}

ADungeonRoom* ARoomGenerator::GenerateRoom(int32 TilesToCreate)
{

	UWorld* World = GetWorld();
    if (!World || !DungeonRoomClass || !GridTileToSpawn) return nullptr;

	//Make the room actor
    ADungeonRoom* Room = World->SpawnActor<ADungeonRoom>(DungeonRoomClass, FTransform::Identity);
    if (!Room) return nullptr;
    Room->Initialize(TilesToCreate);

	//Set variables
	tilesSpawned = 0;
	GridSpacesInRoom.Empty();

	//Spawn tiles based on numTiles
	for(int32 i=TilesToCreate; i > 0; --i)
	{
		//Check to see if any tiles have been spawned yet
		if (tilesSpawned == 0)
		{
			//Spawn a default Tile at 0,0,0
			SpawnTile(SpawnLocation);

		}
		if (GridSpacesInRoom.Num() > 0)
		{
			//Choose a random tile from the GridSpacesInRoom array
			const int32 RandomTileIndex = FMath::RandRange(0, GridSpacesInRoom.Num() - 1);
			AGridSpace* RandomTile = GridSpacesInRoom[RandomTileIndex];

			//Get an array of empty spaces surrounding the tile
			const TArray<FVector> EmptyNeighbors = RandomTile->GetEmptyNeighbors();

			if (EmptyNeighbors.Num() > 0)
			{
				//Choose a random space in the EmptyNeighbors array
				const int32 RandomEmptyNeighborIndex = FMath::RandRange(0, EmptyNeighbors.Num() - 1);
				FVector RandomEmptyNeighbor = EmptyNeighbors[RandomEmptyNeighborIndex];

				//Spawn a tile at the random space's location
				SpawnTile(RandomEmptyNeighbor);
			}
			
		}
	}

	//Attach every spawned tile to the room and register it
	for (AGridSpace* Tile : GridSpacesInRoom)
	{
		if (!Tile) continue;
		Room->AddTile(Tile);
	}

	return Room;
}

