// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomGenerator.generated.h"

class AGridSpace;
class ADungeonRoom;

UCLASS()
class PROCEDURALDUNGEON4_API ARoomGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomGenerator();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Room Generation")
	int numTiles = 1;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	int tilesSpawned = 0;

	UPROPERTY(EditAnywhere, Category="Spawning")
    TSubclassOf<AGridSpace> GridTileToSpawn;

	UPROPERTY(EditAnywhere, Category="Spawning")
    FVector SpawnLocation = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category="Spawning")
    FRotator SpawnRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<AGridSpace*> GridSpacesInRoom;

	UPROPERTY(EditAnywhere, Category="Spawning")
	TSubclassOf<ADungeonRoom> DungeonRoomClass;

	ADungeonRoom* GenerateRoom(int32 TilesToCreate);

	void SpawnTile(const FVector& spawnLocation);

	FActorSpawnParameters SpawnParams;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
