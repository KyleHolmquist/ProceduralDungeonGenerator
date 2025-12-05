// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Holmquist_FloorGenerator.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API AHolmquist_FloorGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHolmquist_FloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// ---- Config ----

	//Dimensions of the grid in cells
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 GridWidth = 20;
	
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 GridHeight = 20;

	//Target number of floor tiles to carve
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int NumTiles = 50;

	//Size of each tile in world units
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	float TileSize = 400.f;

	//Random seed for reproducibility
	UPROPERTY(EditAnywhere, Category = "Room Gen")
	int32 Seed = 12345;

	//Meshes for visualization
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* FloorMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMesh* WallMesh;
	
	//Vertical offset for the floor tiles
	UPROPERTY(EditAnywhere, Category = "Mesh")
	float FloorZ = 0.f;

	//---- Internal Data ----

	//Logical grid - true = floor, false = empty
	TArray<bool> Grid;

	//List of floor cells to grow from
	TArray<FIntPoint> Frontier;

	FORCEINLINE int32 Index(int32 X, int32 Y) const
	{
		return Y * GridWidth + X;
	}

	//---- Pipeline ----

	//Fills the Grid[]
	void GenerateRoomLayout();

	//Spawns floor meshes from the Grid[]
	void SpawnFloorTiles();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
