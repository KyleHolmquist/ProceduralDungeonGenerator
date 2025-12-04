// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Walk_FloorGenerator.generated.h"

UCLASS()
class PROCEDURALDUNGEON4_API AWalk_FloorGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWalk_FloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Grid Size
    UPROPERTY(EditAnywhere, Category = "Walker")
	int32 MapWidth = 60;
	
    UPROPERTY(EditAnywhere, Category = "Walker")
	int32 MapHeight = 40;

	//Number of random walk steps to take
    UPROPERTY(EditAnywhere, Category = "Walker")
	int32 NumSteps = 1000;

	//Bool to start in center or random start
    UPROPERTY(EditAnywhere, Category = "Walker")
	bool bStartInCenter = true;

	//Size of a tile in world units
    UPROPERTY(EditAnywhere, Category = "Walker")
	float TileSize = 100.f;

	//Mesh to use for floor cells
    UPROPERTY(EditAnywhere, Category = "Walker")
	UStaticMesh* FloorMesh;

	//Wall mesh
    UPROPERTY(EditAnywhere, Category = "Walker")
	UStaticMesh* WallMesh;

	//Height offsets
    UPROPERTY(EditAnywhere, Category = "Walker")
	float FloorZ = 0.f;

    UPROPERTY(EditAnywhere, Category = "Walker")
	float WallHeight = 200.f;	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//true = wall, false= floor
	TArray<bool> Map;

	FORCEINLINE int32 Index(int32 X, int32 Y) const
	{
		return Y * MapWidth + X;
	};

	void GenerateMap();
	void InitializeMap();
	void RunRandomWalk();
	void SpawnGeometry();

	bool HasFloorNeighbor(int32 X, int32 Y) const;

};
