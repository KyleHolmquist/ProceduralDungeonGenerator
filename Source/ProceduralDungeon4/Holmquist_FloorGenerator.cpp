// Fill out your copyright notice in the Description page of Project Settings.


#include "Holmquist_FloorGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AHolmquist_FloorGenerator::AHolmquist_FloorGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void AHolmquist_FloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	//Allocate Grid
	GenerateRoomLayout();
	SpawnFloorTiles();
	
}

// Called every frame
void AHolmquist_FloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHolmquist_FloorGenerator::GenerateRoomLayout()
{
	//Clear state
	const int32 NumCells = GridWidth * GridHeight;
	if (NumCells <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: Invalid grid size"));
		return;
	}

	Grid.Init(false, NumCells);
	Frontier.Reset();

	//RNG Setup
	FRandomStream Rng;
	if (Seed >= 0)
	{
		Rng.Initialize(Seed);
	}
	else
	{
		//Non-deterministic
		Rng.GenerateNewSeed();
	}

	//Choose starting cell at center of grid
	const int32 StartX = GridWidth / 2;
	const int32 StartY = GridHeight / 2;

	Grid[Index(StartX, StartY)] = true;
	Frontier.Add(FIntPoint(StartX, StartY));

	int32 TilesPlaced = 1;
	const int32 TargetTiles = FMath::Clamp(NumTiles, 1, NumCells);

	//Grow the room
	while (TilesPlaced < TargetTiles && Frontier.Num() > 0)
	{
		//Choose a random floor cell to grow from
		const int32 FrontierIndex = Rng.RandRange(0, Frontier.Num() - 1);
		const FIntPoint Cell = Frontier[FrontierIndex];
		
		const int32 X = Cell.X;
		const int32 Y = Cell.Y;

		//Gather empty neighbors around the cell
		TArray<FIntPoint> EmptyNeighbors;

		const int32 DX[4] = {1, -1, 0, 0};
		const int32 DY[4] = {0, 0, 1, -1};

		for (int32 i = 0; i < 4; ++i)
		{
			const int32 NX = X + DX[i];
			const int32 NY = Y + DY[i];

			//Stay inside the grid
			if (NX < 0 || NX >= GridWidth || NY < 0 || NY >= GridHeight) continue;

			if (!Grid[Index(NX, NY)])
			{
				EmptyNeighbors.Add(FIntPoint(NX, NY));
			}
		}

		if (EmptyNeighbors.Num() == 0)
		{
			//This floor cell has no empty neighbors. Remove from Frontier array
			Frontier.RemoveAtSwap(FrontierIndex);
			continue;
		}

		//Choose a random empty neighbor
		const int32 RandEmptyIndex = Rng.RandRange(0, EmptyNeighbors.Num() - 1);
		const FIntPoint NewCell = EmptyNeighbors[RandEmptyIndex];

		//Carve floor
		Grid[Index(NewCell.X, NewCell.Y)] = true;
		Frontier.Add(NewCell);

		++TilesPlaced;
	}

	UE_LOG(LogTemp, Log, TEXT("Holmquist_FloorGenerator: Placed %d floor tiles (target %d)."),
			TilesPlaced, TargetTiles);
}

void AHolmquist_FloorGenerator::SpawnFloorTiles()
{
	if (!FloorMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("Holmquist_FloorGenerator: FloorMesh not assigned."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World) return;

	const float BaseSize = 100.f;

	for (int32 y = 0; y < GridHeight; ++y)
	{
		for (int32 x = 0; x < GridWidth; ++x)
		{
			if (!Grid[Index(x, y)]) continue;

			const FVector Loc = GetActorLocation() + FVector(x * TileSize, y * TileSize, FloorZ);

			AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(Loc, FRotator::ZeroRotator);
			if (!FloorActor) continue;

			UStaticMeshComponent* MeshComp = FloorActor->GetStaticMeshComponent();
			if (!MeshComp)
			{
				FloorActor->Destroy();
				continue;
			}

			MeshComp->SetStaticMesh(FloorMesh);
			
			const float Scale = TileSize / BaseSize;
			FloorActor->SetActorScale3D(FVector(Scale, Scale, 1.f));
			FloorActor->SetMobility(EComponentMobility::Static);
		}
	}
}

